/* sedispatch.c --
 * Copyright 2009 Red Hat Inc., Durham, North Carolina.
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors:
 *   Dan Walsh <dwalsh@redhat.com>
 *   Steve Grubb <sgrubb@redhat.com>
 *
 * This program is an audit dispatcher plugin that searches for AVC messages
 * and sends them as a dbus message to setroubleshootd
 *
 * ausearch --start today --raw > test.log.
 *
 * Then you can test this app by: cat test.log | sedispatch
 *
 *
 */

/* Modified by Hramchenko Vitaliy <v.hramchenko@gmail.com> for using with
 * User Data Defence
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include "libaudit.h"
#include "auparse.h"
#include "uddbus.h"
#include <dbus/dbus.h>
#include <syslog.h>
#include <selinux/context.h>

/* Global Data */
static volatile int stop = 0;
static volatile int hup = 0;
static auparse_state_t *au = NULL;

static DBusConnection* dbusconn = NULL;

/* Local declarations */
static void handle_event(auparse_state_t *au, auparse_cb_event_t cb_event_type,
    void *user_data);

/*
 * SIGTERM handler
 */
static void term_handler(int sig __attribute__((unused))) {
  stop = 1;
}

/*
 * SIGHUP handler: re-read config
 */
static void hup_handler(int sig __attribute__((unused))) {
  hup = 1;
}

static void reload_config(void) {
  hup = 0;
}

static DBusConnection* init_dbus(void) {

  DBusError err;
  DBusConnection* conn = NULL;
  // initialise the errors
  dbus_error_init(&err);

  // connect to the bus
  conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
  if (dbus_error_is_set(&err)){
    syslog(LOG_ERR, "UDD Connection Error (%s): AVC Will be dropped\n", err.message);
    dbus_error_free(&err);
  }
  return conn;
}

int main(int argc __attribute__((unused)), char *argv[] __attribute__((unused))) {
  syslog(LOG_ERR, "Starting UDDBus");
  char tmp[MAX_AUDIT_MESSAGE_LENGTH + 1];
  struct sigaction sa;

  /* Register sighandlers */
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);
  /* Set handler for the ones we care about */
  sa.sa_handler = term_handler;
  sigaction(SIGTERM, &sa, NULL);
  sa.sa_handler = hup_handler;
  sigaction(SIGHUP, &sa, NULL);

  /* Initialize the auparse library */
  au = auparse_init(AUSOURCE_FEED, 0);
  if (au == NULL){
    syslog(LOG_ERR, "UDDBus is exiting due to auparse init errors");
    return -1;
  }

  auparse_add_callback(au, handle_event, NULL, NULL);

  do{
    /* Load configuration */
    if (hup){
      reload_config();
    }

    /* Now the event loop */
    while (fgets_unlocked(tmp, MAX_AUDIT_MESSAGE_LENGTH, stdin) && hup == 0
        && stop == 0){
      auparse_feed(au, tmp, strnlen(tmp, MAX_AUDIT_MESSAGE_LENGTH));
    }
    if (feof(stdin))
      break;
  }while (stop == 0);

  /* Flush any accumulated events from queue */
  auparse_flush_feed(au);
  auparse_destroy(au);
  if (stop)
    syslog(LOG_DEBUG, "UDDbus is exiting on stop request\n");
  else
    syslog(LOG_ERR, "UDDBus is exiting on stdin EOF\n");

  return 0;
}

static int is_setroubleshoot(const char *context) {
  int ret = FALSE;
  if (context){
    context_t con = context_new(context);
    const char* type = context_type_get(con);
    ret = (strcmp(type, "user_data_defence_bus_t") == 0);
    context_free(con);
  }
  return ret;
}

/* This function shows how to dump a whole record's text */
static void dump_whole_record(auparse_state_t *au, void *conn) {
  char *tmp = NULL;
  int len = 0;
  const char *scon = auparse_find_field(au, "scontext");
  const char *tcon = auparse_find_field(au, "tcontext");
  if (is_setroubleshoot(scon) || is_setroubleshoot(tcon)){
    syslog(LOG_ERR, "AVC Message for UDDaemon, dropping message");
    return;
  }

  auparse_first_record(au);
  do{
    len = asprintf(&tmp, "%s%s\n", tmp, auparse_get_record_text(au));
    if (len < 0){
      syslog(LOG_ERR, "UDDBus out of memory\n");
      free(tmp);
      return;
    }
  }while (auparse_next_record(au) > 0);

  if (!dbusconn){
    dbusconn = init_dbus();
  }
  if (dbusconn){
    uddbus_send_avc(dbusconn, tmp);
  }
  else
    syslog(LOG_ERR, "UDDBus error. Connection not initialized.\n");

  free(tmp);
}

/* This function receives a single complete event at a time from the auparse
 * library. This is where the main analysis code would be added. */
static void handle_event(auparse_state_t *au, auparse_cb_event_t cb_event_type,
    void *user_data) {
  int type, num = 0;

  DBusConnection* conn = (DBusConnection*)user_data;

  if (cb_event_type != AUPARSE_CB_EVENT_READY)
    return;

  /* Loop through the records in the event looking for one to process.
   We use physical record number because we may search around and
   move the cursor accidentally skipping a record. */
  while (auparse_goto_record_num(au, num) > 0){
    type = auparse_get_type(au);
    /* Now we can branch based on what record type we find.
     This is just a few suggestions, but it could be anything. */
    switch (type){
    case AUDIT_AVC:
      dump_whole_record(au, conn);
      break;
    case AUDIT_SYSCALL:
      break;
    case AUDIT_USER_LOGIN:
      break;
    case AUDIT_ANOM_ABEND:
      break;
    case AUDIT_MAC_STATUS:
      break;
    default:
      break;
    }
    num++;
  }
}

