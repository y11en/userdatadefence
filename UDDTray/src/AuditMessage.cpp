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

#include "AuditMessage.h"

#include <QtDebug>

#include "AuditMessageNotificationSettings.h"
#include "functions.h"

AuditMessage::AuditMessage() {
  _timeStamp = new AuditMessageTimeStamp();
}

AuditMessage::~AuditMessage() {
  clear();
  delete _timeStamp;
}

void AuditMessage::clear(){
  clearMapOfPointers<QString, AuditMessageField*>(fields);
}

AuditMessageTimeStamp* AuditMessage::timeStamp(){
  return _timeStamp;
}


void AuditMessage::setMessageFromAuparse(const char* message){
  QString raw_message = QString::fromLocal8Bit(message);
  if (!raw_message.startsWith("type="))
    raw_message = raw_message.mid(raw_message.indexOf(" ") + 1);
  setRawMessage(raw_message);
}

QString AuditMessage::rawMessage(){
  return _rawMessage;
}

void AuditMessage::setRawMessage(QString text){
  _rawMessage = text;
  _notificationMessage = AuditMessageNotificationSettings::currentSettings()->searchNotificationMessage(this);
}

QIcon AuditMessage::icon(){
  return _notificationMessage->icon();
}

QByteArray AuditMessage::pixmapBytes(){
  return _notificationMessage->pixmapBytes();
}

QString AuditMessage::formattedMessage(){
  return _notificationMessage->formatMessage(this);
}

QString AuditMessage::caption(){
  return _notificationMessage->caption();
}

AuditMessageNotification* AuditMessage::messageNotification(){
  return _notificationMessage;
}

bool AuditMessage::needNotification(){
  return _notificationMessage->needNotification();
}
