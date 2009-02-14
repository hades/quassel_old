/***************************************************************************
*   Copyright (C) 2005-09 by the Quassel Project                          *
*   devel@quassel-irc.org                                                 *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) version 3.                                           *
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

#ifndef SYSTEMTRAY_H_
#define SYSTEMTRAY_H_

#include <QSystemTrayIcon>
#include <QTimer>

#include "icon.h"

class SystemTray : public QObject {
  Q_OBJECT

public:
  enum State {
    Inactive,
    Active
  };

  SystemTray(QObject *parent = 0);
  ~SystemTray();

  inline bool isSystemTrayAvailable() const;
  Icon icon() const;
  QString toolTip() const;

public slots:
  void setState(State);
  void setAlert(bool alert = true);
  void setIconVisible(bool visible = true);
  void setToolTip(const QString &tip);
  void showMessage(const QString &title, const QString &message,
                   QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::Information, int millisecondsTimeoutHint = 10000);


signals:
  void activated(QSystemTrayIcon::ActivationReason);
  void iconChanged(const Icon &);
  void messageClicked();

private slots:
  void nextPhase();

private:
  void loadAnimations();

  QSystemTrayIcon *_trayIcon;
  QMenu *_trayMenu;
  State _state;
  bool _alert;

  int _idxOffStart, _idxOffEnd, _idxOnStart, _idxOnEnd, _idxAlertStart;
  int _currentIdx;
  QTimer _animationTimer;

  QList<QPixmap> _phases;
};

// inlines

bool SystemTray::isSystemTrayAvailable() const { return QSystemTrayIcon::isSystemTrayAvailable(); }

#endif
