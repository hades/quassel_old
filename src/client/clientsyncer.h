/***************************************************************************
 *   Copyright (C) 2005-08 by the Quassel IRC Team                         *
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

#ifndef _CLIENTSYNCER_H_
#define _CLIENTSYNCER_H_

#include <QPointer>
#include <QString>
#include <QTcpSocket>
#include <QVariantMap>

class IrcUser;
class IrcChannel;

class ClientSyncer : public QObject {
  Q_OBJECT

  public:
    ClientSyncer(QObject *parent = 0);
    ~ClientSyncer();

  signals:
    void recvPartialItem(quint32 avail, quint32 size);
    void connectionError(const QString &errorMsg);
    void connectionMsg(const QString &msg);
    void sessionProgress(quint32 part, quint32 total);
    void networksProgress(quint32 part, quint32 total);
    void channelsProgress(quint32 part, quint32 total);
    void ircUsersProgress(quint32 part, quint32 total);
    void socketStateChanged(QAbstractSocket::SocketState);
    void socketDisconnected();

    void startLogin();
    void loginFailed(const QString &error);
    void loginSuccess();
    void syncFinished();


  public slots:
    void connectToCore(const QVariantMap &);
    void loginToCore(const QString &user, const QString &passwd);
    void disconnectFromCore();

  private slots:
    void coreSocketError(QAbstractSocket::SocketError);
    void coreHasData();
    void coreSocketConnected();
    void coreSocketDisconnected();

    void clientInitAck(const QVariantMap &msg);

  // for sync progress
    void networkInitDone();
    void ircUserAdded(IrcUser *);
    void ircUserRemoved(QObject *);
    void ircUserInitDone(IrcUser *);
    void ircChannelAdded(IrcChannel *);
    void ircChannelRemoved(QObject *);
    void ircChannelInitDone(IrcChannel *);
    void checkSyncState();

    void syncToCore(const QVariantMap &sessionState);
    void sessionStateReceived(const QVariantMap &state);

  private:
    QPointer<QIODevice> socket;
    quint32 blockSize;
    QVariantMap coreConnectionInfo;

    QSet<QObject *> netsToSync, channelsToSync, usersToSync;
    int numNetsToSync, numChannelsToSync, numUsersToSync;

};

#endif