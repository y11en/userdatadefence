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

#include "AuditMessageRegExpFilterNotification.h"

#include "AuditMessage.h"

AuditMessageRegExpFilterNotification::AuditMessageRegExpFilterNotification(AuditMessageNotificationSettings* notification_settings): AuditMessageNotification(notification_settings){
}

AuditMessageRegExpFilterNotification::~AuditMessageRegExpFilterNotification() {
}

bool AuditMessageRegExpFilterNotification::processExtraAttributes(){
  setFilterString(readXMLAttribute("regexp"));
  return true;
}

bool AuditMessageRegExpFilterNotification::checkNotification(AuditMessage* message){
  if (message->rawMessage().contains(_regExpr))
    return true;
  else
    return false;
}

void AuditMessageRegExpFilterNotification::setFilterString(QString filter){
  _filterString = filter;
  _regExpr = QRegExp(filterString());
}

QString AuditMessageRegExpFilterNotification::elementName(){
  return "event_pattern";
}

void AuditMessageRegExpFilterNotification::writeExtraAttributes(QXmlStreamWriter* xml_writer){
  xml_writer->writeAttribute("regexp", filterString());
}

