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

#include "TrayIcon.h"

#include "MainWindow.h"
#include "ApplicationsParser.h"
#include "AlertsWatcher.h"
#include "AlertsStatistics.h"

#include <QtDebug>

TrayIcon* TrayIcon::_instance = 0;

TrayIcon::TrayIcon(): QSystemTrayIcon(){
  mainWindow = MainWindow::instance();
}

TrayIcon::~TrayIcon() {
  delete menu;
}

TrayIcon* TrayIcon::instance(){
  if (!_instance)
    _instance = new TrayIcon();
  return _instance;
}

void TrayIcon::init(){
  createMenu();
  setContextMenu(menu);
  setToolTip("<b>User Data Defence</b>");
  connect(this, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(iconActivatedST(QSystemTrayIcon::ActivationReason)));
  connect(mainWindow, SIGNAL(visibleChanged(bool)), this, SLOT(mainWindowVisibleST(bool)));
  if (AlertsStatistics::instance()->alertsCount)
    setMode("alert", 0);
  else
    setMode("default", 0);
}

void TrayIcon::reloadMenu(){
  delete menu;
  createMenu();
  setContextMenu(menu);
}

void TrayIcon::createMenu(){
  menu = new QMenu();

  loadControllableApplications();

  showHideAction = new QAction("Show", menu);
  connect(showHideAction, SIGNAL(triggered()), mainWindow, SLOT(changeVisibleST()));

  QAction* quit_action = new QAction("&Quit", menu);
  connect(quit_action, SIGNAL(triggered()), mainWindow, SLOT(quitST()));

  menu->addAction(showHideAction);
  menu->addAction(quit_action);
}


void TrayIcon::setMode(QString mode, int){
  if (mode == "default"){
    QString pixmap_name = ":/images/logo2.png";
    setIcon(QPixmap(pixmap_name));
  }
  if (mode == "alert" && (MainWindow::instance()->isVisible() == false)){
    QString pixmap_name = ":/images/logo2_alert.png";
    setIcon(QPixmap(pixmap_name));
  }
}

void TrayIcon::mainWindowVisibleST(bool is_visible){
  QString text = "Show";
  if (is_visible){
    text = "Hide";
    setMode("default", 0);
  }
  showHideAction->setText(text);
}

void TrayIcon::iconActivatedST(QSystemTrayIcon::ActivationReason reason){
  if(reason == QSystemTrayIcon::Trigger){
    mainWindow->changeVisibleST();
  }
}

void TrayIcon::loadControllableApplications(){
  ApplicationsControlRoot::currentApplications()->appendMenuItems(menu);
}
