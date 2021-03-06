/***************************************************************************
 *   Copyright (C) 2011 by Hramchenko                                      *
 *   v.hramchenko@gmail.com                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 ***************************************************************************/

#include "ApplicationSettingsBox.h"

#include "ApplicationsMenuItem.h"

#include <QtDebug>

ApplicationSettingsBox::ApplicationSettingsBox(QWidget* parent): QWidget(parent) {
  setupUi(this);
}

ApplicationSettingsBox::~ApplicationSettingsBox() {
}

void ApplicationSettingsBox::parameterSelectedST(ApplicationsSettingsParameter* parameter){
  _menuItem = (ApplicationsMenuItem*)parameter;
  if (!_menuItem)
    return;
  nameLE->setText(_menuItem->name());
  textLE->setText(_menuItem->text);
  imageSelectionBox->loadImageST(_menuItem->iconFile);
}

void ApplicationSettingsBox::nameChangedST(QString text){
  if (!_menuItem)
    return;
  _menuItem->setName(text);
  emit(parameterNameChanged(_menuItem));
}

void ApplicationSettingsBox::textChangedST(QString text){
  if (!_menuItem)
    return;
  _menuItem->text = text;
}

void ApplicationSettingsBox::iconChangedST(QString text){
  if (!_menuItem)
    return;
  _menuItem->setIcon(text);
}

void ApplicationSettingsBox::setNameEditorLabel(QString text){
  nameLB->setText(text);
}
