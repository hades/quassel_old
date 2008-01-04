/***************************************************************************
 *   Copyright (C) 2005-08 by the Quassel IRC Team                         *
 *   devel@quassel-irc.org                                                 *
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
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef _IDENTITIESSETTINGSPAGE_H_
#define _IDENTITIESSETTINGSPAGE_H_

#include "settingspage.h"

#include "ui_identitiessettingspage.h"

class IdentitiesSettingsPage : public SettingsPage {
  Q_OBJECT

  public:
    IdentitiesSettingsPage(QWidget *parent = 0);

    bool hasChanged() const;

  public slots:
    void save();
    void load();
    void defaults();

  private:
    Ui::IdentitiesSettingsPage ui;

};

#endif