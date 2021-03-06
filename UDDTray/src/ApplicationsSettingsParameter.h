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

#ifndef APPLICATIONSSETTINGSPARAMETER_H_
#define APPLICATIONSSETTINGSPARAMETER_H_

#include "XMLFileElement.h"

#include <QString>
#include <QList>


class ApplicationsSettingsParameter: public XMLFileElement {
public:
  ApplicationsSettingsParameter();
  virtual ~ApplicationsSettingsParameter();
  virtual QString name();
  virtual void setName(QString name);
  virtual QList<ApplicationsSettingsParameter*>* subElements();
  virtual ApplicationsSettingsParameter* appendNewItem();
private:
  QString _name;
};

#endif /* APPLICATIONSSETTINGSPARAMETER_H_ */
