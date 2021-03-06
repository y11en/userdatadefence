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
#include "UDDBusServer.h"

#include <QtDebug>

UDDBusServer::UDDBusServer(QObject *obj): QDBusAbstractAdaptor(obj){
}

UDDBusServer::~UDDBusServer(){
}

void UDDBusServer::AVCMessage(const QString &message){
  if (message == lastMessage)
    return;
  lastMessage = message;
  emit(messageReceived(message));
}

QString UDDBusServer::readFileFromLine(QString file_name, QString line){
  QFile file(file_name);
  file.open(QIODevice::ReadOnly);
  QTextStream input_stream(&file);
  QString current_line;
  bool line_not_found = true;
  if (!line.isEmpty()){
    while (!input_stream.atEnd()){
      current_line = input_stream.readLine();
      if (current_line == line){
        line_not_found = false;
        break;
      }
    }
  }
  QString result;
  if (line_not_found){
    input_stream.seek(0);
  }
  if (!input_stream.atEnd())
    result = input_stream.readAll();
  file.close();
  return result;
}

QString UDDBusServer::getAuditMessages(QString last_message){
  QString result;
  result = readFileFromLine("/var/log/audit/audit.log", last_message);
  return result;
}

QString UDDBusServer::getMessages(QString last_message){
  QString result;
  result = readFileFromLine("/var/log/messages", last_message);
  return result;
}

QString UDDBusServer::getLastMessagesLines(int lines_count){
  QFile log("/var/log/messages");
  log.open(QIODevice::ReadOnly);
  QTextStream log_stream(&log);
  QString line;
  QStringList last_lines;
  int current_count = 0;
  while (!log_stream.atEnd()){
    line = log_stream.readLine();
    last_lines.push_back(line);
    if (current_count >= lines_count)
      last_lines.pop_front();
    else
      current_count++;
  }
  log.close();
  QString result;
  result = last_lines.join("\n");
  return result;
}
