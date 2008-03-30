/***************************************************************************
 *   Copyright (C) 2005-08 by the Quassel Project                          *
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

#ifndef BUFFERVIEWMANAGER_H
#define BUFFERVIEWMANAGER_H

#include "syncableobject.h"

#include <QList>
#include <QHash>

class BufferViewConfig;
class SignalProxy;

class BufferViewManager : public SyncableObject {
  Q_OBJECT

public:
  BufferViewManager(SignalProxy *proxy, QObject *parent = 0);

  inline QList<BufferViewConfig *> bufferViewConfigs() const { return _bufferViewConfigs.values(); }
  BufferViewConfig *bufferViewConfig(int bufferViewId) const;

public slots:
  void addBufferViewConfig(BufferViewConfig *config);
  void addBufferViewConfig(int bufferViewConfigId);
  inline void newBufferViewConfig(int bufferViewConfigId)  { addBufferViewConfig(bufferViewConfigId); }

  QVariantList initBufferViewIds() const;
  void initSetBufferViewIds(const QVariantList bufferViewIds);

  virtual inline void requestCreateBufferView(const QString &bufferViewName) { emit createBufferViewRequested(bufferViewName); }

signals:
  void bufferViewConfigAdded(int bufferViewConfigId);
  void createBufferViewRequested(const QString &bufferViewName);

protected:
  typedef QHash<int, BufferViewConfig *> BufferViewConfigHash;
  inline const BufferViewConfigHash &bufferViewConfigHash() { return _bufferViewConfigs; }

private:
  BufferViewConfigHash _bufferViewConfigs;
  SignalProxy *_proxy;
};

#endif // BUFFERVIEWMANAGER_H