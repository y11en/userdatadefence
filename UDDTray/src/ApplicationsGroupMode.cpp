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

#include "ApplicationsGroupMode.h"

#include <QMenu>
#include <QAction>

#include "ApplicationsGroup.h"
#include "UDDExecBus.h"
#include "ControllableApplication.h"
#include "functions.h"

#include <QtDebug>

ApplicationsGroupMode::ApplicationsGroupMode(ApplicationsGroup* group) {
  _group = group;
}

ApplicationsGroupMode::~ApplicationsGroupMode() {
  clearListOfPointers<ApplicationsSettingsParameter*>(applications);
}

bool ApplicationsGroupMode::processExtraAttributes(){
  return true;
}

XMLFileElement* ApplicationsGroupMode::newElement(const QString& name, bool& error_flag){
  if (name != "application"){
    error_flag = true;
    return 0;
  }
  return appendNewItem();
}

QString ApplicationsGroupMode::elementName(){
  return "group_mode";
}


void ApplicationsGroupMode::writeSubElements(QXmlStreamWriter* xml_writer){
  QListIterator<ApplicationsSettingsParameter*> iter(applications);
  ControllableApplication* app;
  while (iter.hasNext()){
    app = (ControllableApplication*)iter.next();
    app->writeElement(xml_writer);
  }
}

void ApplicationsGroupMode::setActive(bool value){
  QFont font = trayAction->font();
  font.setBold(value);
  trayAction->setFont(font);
  if (value)
    loadCommands();
}

void ApplicationsGroupMode::loadCommands(){
  UDDExecBus* bus = UDDExecBus::instance();
  QListIterator<ApplicationsSettingsParameter*> iter(applications);
  while(iter.hasNext()){
    ControllableApplication* application = (ControllableApplication*)iter.next();
    bus->commandsDict[application->name()] = application->command;
  }
}


void ApplicationsGroupMode::modeSelectedST(){
  setActive(true);
  QListIterator<ApplicationsSettingsParameter*> iter(_group->modes);
  while(iter.hasNext()){
    ApplicationsGroupMode* mode = (ApplicationsGroupMode*)iter.next();
    if (mode != this)
      mode->setActive(false);
  }
}

void ApplicationsGroupMode::appendMenuItems(QMenu* menu){
  trayAction = new QAction(text, menu);
  if (icon)
    trayAction->setIcon(*icon);
  connect(trayAction, SIGNAL(triggered()), this, SLOT(modeSelectedST()));
  menu->addAction(trayAction);
}

QList<ApplicationsSettingsParameter*>* ApplicationsGroupMode::subElements(){
  return &applications;
}

ApplicationsSettingsParameter* ApplicationsGroupMode::appendNewItem(){
  ControllableApplication* application = new ControllableApplication();
  applications.push_back(application);
  return application;
}
