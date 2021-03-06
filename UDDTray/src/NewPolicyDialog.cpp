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

#include "NewPolicyDialog.h"

#include <QFileInfo>
#include <QFile>
#include <QTextStream>
#include <QRegExp>
#include <QFileDialog>

#include "functions.h"
#include "GlobalSettings.h"

#include <QtDebug>

NewPolicyDialog::NewPolicyDialog(QWidget* parent): QDialog(parent){
  setupUi(this);
  QRegExp rx("(\\w)*");
  _nameValidator = new QRegExpValidator(rx, this);
  nameLE->setValidator(_nameValidator);
  projectTypeLW->setCurrentRow(0);
}

NewPolicyDialog::~NewPolicyDialog(){
}

void NewPolicyDialog::createPolicyST(){
  createFileFromTemplate(".te");
  createFileFromTemplate(".fc");
  createFileFromTemplate(".if");
  createMakefile();
  close();
}

void NewPolicyDialog::createMakefile(){
  createIfNeed("Makefile");
  createIfNeed("udd.if");
  createIfNeed("udd.te");
  createIfNeed("udd.fc");
}

void NewPolicyDialog::createIfNeed(QString file_name){
  QString file_path = _folder + "/" + file_name;
  QFileInfo file_info(file_path);
  if (!file_info.exists()){
    QString file_template = GlobalSettings::instance()->getConfigFile("templates/" + file_name);
    QFile::copy(file_template, file_path);
  }
}

QString NewPolicyDialog::policyFilePath(){
  return _folder + "/" + _name;
}

void NewPolicyDialog::createFileFromTemplate(QString suffix){
  policyFile = "";
  QFile template_file(templateFile(suffix));
  if (!template_file.open(QIODevice::ReadOnly))
    return;
  QTextStream template_stream(&template_file);
  QString data;
  data = template_stream.readAll();
  template_file.close();
  data = data.replace("POLICYNAME", _name);
  QFile target_file(policyFilePath() + suffix);
  if (!target_file.open(QIODevice::WriteOnly))
    return;
  QTextStream target_stream(&target_file);
  target_stream<<data;
  target_file.close();
  policyFile = policyFilePath();
}

QString NewPolicyDialog::templateFile(QString suffix){
  int index = projectTypeLW->currentRow();
  QString result;
  if (index < 1)
    result = "empty_policy";
  else if (index == 1)
    result = "console_policy";
  else if (index == 2)
    result = "gui_policy";
  result = result + suffix;
  result = GlobalSettings::instance()->getConfigFile("templates/" + result);
  return result;
}

void NewPolicyDialog::cancelST(){
  close();
}

void NewPolicyDialog::selectFolderST(){
  QString file_name;
  file_name = QFileDialog::getExistingDirectory (this, tr("Select policy directory"), tr("./"));
  folderLE->setText(file_name);
}

void NewPolicyDialog::nameChangedST(QString name){
  _name = name;
  checkCreation();
}

void NewPolicyDialog::folderChangedST(QString folder){
  _folder = folder;
  checkCreation();
}

bool NewPolicyDialog::checkFile(bool old_flag, QString suffix){
  QFileInfo info(policyFilePath() + suffix);
  old_flag = old_flag && (!info.exists());
  return old_flag;
}

void NewPolicyDialog::checkCreation(){
  bool flg = true;
  QFileInfo folder_info(_folder);
  flg = flg && folder_info.isDir();
  flg = flg && folder_info.isWritable();
  flg = flg && (!_name.isEmpty());
  flg = checkFile(flg, ".te");
  flg = checkFile(flg, ".if");
  flg = checkFile(flg, ".fc");
  createBTN->setEnabled(flg);
}
