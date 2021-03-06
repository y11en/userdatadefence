/***************************************************************************
 *   Copyright (C) 2011 by Hramchenko                                      *
 *   v.hramchenko@gmail.com                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 ***************************************************************************/
#include "KNotification.h"

#include <QDBusInterface>
#include <QBuffer>
#include <QDBusReply>
#include <QObject>
#include <QtDebug>

KNotification* KNotification::_instance = 0;

KNotification::KNotification(): QObject(){
  connectToInterface();
  notifyEvent = "alert";
  notifyApplication = "UDDTray";
  _queueLength = 0;
  notificationTimer = new QTimer();
  connect(notificationTimer, SIGNAL(timeout()), this, SLOT(sendMessagesFromQueueST()));
}

KNotification::~KNotification() {
  clearMessagesDict();
  delete knotifyInterface;
  delete notificationTimer;
}

KNotification* KNotification::instance(){
  if (!_instance)
    _instance = new KNotification();
  return _instance;
}

void KNotification::clearMessagesDict(){
  QMapIterator<int, KNotificationMessage*> iter(messagesDict);
  while (iter.hasNext()){
    iter.next();
    delete iter.value();
  }
  messagesDict.clear();
}

void KNotification::connectToInterface(){
  knotifyInterface = new QDBusInterface("org.kde.knotify", "/Notify", "org.kde.KNotify");
  connect(knotifyInterface, SIGNAL(notificationActivated(int, int)), this, SLOT(notificationActivated(int, int)));
  connect(knotifyInterface, SIGNAL(notificationClosed(int)), this, SLOT(notificationClosed(int)));
}

void KNotification::notificationActivated(int id, int action){
  KNotificationMessage* message;
  message = findNotificationMessage(id);
  if (message){
    message->actionActivated(action);
    removeNotificationMessage(id);
  }
}

void KNotification::notificationClosed(int id){
  removeNotificationMessage(id);
}

void KNotification::removeNotificationMessage(int id){
  KNotificationMessage* message;
  message = findNotificationMessage(id);
  if (message){
    message->notificationClosed();
    if (message->needAutoDeletion()){
      delete message;
    }
    messagesDict.remove(id);
  }
}

void KNotification::closeMessage(KNotificationMessage* message){
  QList<QVariant> message_args;
  message_args.push_back(message->messageId);
  knotifyInterface->callWithArgumentList(QDBus::AutoDetect, "closeNotification", message_args);
}

void KNotification::closeAllMessages(){
  QMapIterator<int, KNotificationMessage*> iter(messagesDict);
  while (iter.hasNext()){
    iter.next();
    closeMessage(iter.value());
  }
}


void KNotification::updateMessage(KNotificationMessage* message){
  if (messagesDict.contains(message->messageId))
    sendUpdateMessageToDBus(message);
  else
    showMessage(message);
}

void KNotification::sendUpdateMessageToDBus(KNotificationMessage* message){
  QList<QVariant>* message_args;
  message_args = createUpdateEventArgs(message);
  knotifyInterface->callWithArgumentList(QDBus::AutoDetect, "update", *message_args);
  message->notificationShown();
  delete message_args;
}

QList<QVariant>* KNotification::createUpdateEventArgs(KNotificationMessage* message){
  QList<QVariant>* message_args = new QList<QVariant>;
  message_args->append(message->messageId);
  message_args->append(message->title());
  message_args->append(message->text());
  message_args->append(message->pixmapBytes());
  message_args->append(QVariant(message->actions()));
  return message_args;
}

KNotificationMessage* KNotification::findNotificationMessage(int id){
  KNotificationMessage* result = 0;
  if (messagesDict.contains(id)){
    result = messagesDict[id];
  }
  return result;
}

int KNotification::sendMessageToDBus(KNotificationMessage* message){
  message->notificationShown();
  QList<QVariant>* message_args;
  message_args = createEventArgs(message);
  QDBusReply<int> reply;
  reply = knotifyInterface->callWithArgumentList(QDBus::AutoDetect, "event", *message_args);
  message->messageId = reply.value();
  delete message_args;
  return message->messageId;
}

QList<QVariant>* KNotification::createEventArgs(KNotificationMessage* message){
  QList<QVariant>* message_args = new QList<QVariant>;
  message_args->append(notifyEvent);
  message_args->append(notifyApplication);
  message_args->append(QVariant::List);
  message_args->append(message->title());
  message_args->append(message->text());
  message_args->append(message->pixmapBytes());
  message_args->append(QVariant(message->actions()));
  message_args->append(message->timeout());
  message_args->append(QVariant::LongLong);
  return message_args;
}

void KNotification::showMessage(KNotificationMessage* message){
  messagesQueue.push_back(message);
  if (message->needQueue()){
    notificationTimer->start(700);
    _queueLength++;
  }
  else{
    int message_id;
    message_id = sendMessageToDBus(message);
    messagesDict[message_id] = message;
  }
}

void KNotification::sendMessagesFromQueueST(){
  KNotificationMessage* message = messagesQueue.takeFirst();
  int message_id;
  message_id = sendMessageToDBus(message);
  messagesDict[message_id] = message;
  if (messagesQueue.isEmpty())
    notificationTimer->stop();
  _queueLength--;
}

int KNotification::queueLength(){
  return messagesDict.count() + _queueLength;
}

void KNotification::flushQueue(){
  notificationTimer->stop();
  _queueLength = 0;
  KNotificationMessage* message;
  while (!messagesQueue.isEmpty()){
    message = messagesQueue.takeFirst();
    if (message->needAutoDeletion())
      delete message;
  }
}

