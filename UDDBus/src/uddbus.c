/*
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
 *
 * Based off Example low-level D-Bus code.
 * Written by Matthew Johnson <dbus@matthew.ath.cx>
 *
 */
/* Modified by Hramchenko Vitaliy <v.hramchenko@gmail.com> for using with
 * User Data Defence
 */

#include <dbus/dbus.h>
#include <stdbool.h>
#include <syslog.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int uddbus_send_avc(DBusConnection* connection, char *avc) {
  DBusError error;
  DBusMessage *call;
  dbus_error_init(&error);
  call = dbus_message_new_method_call("net.sourceforge.userdatadefence.avcbus", "/AVCBus", "net.sourceforge.userdatadefence.avcbus", "AVCMessage");
  dbus_message_append_args (call, DBUS_TYPE_STRING, &avc, DBUS_TYPE_INVALID);
  if (!dbus_connection_send(connection, call, NULL)) {
    syslog(LOG_ERR, "UDD error: Can't send message.\n");
  }
  dbus_connection_flush(connection);
  dbus_message_unref(call);
  return 0;
}

