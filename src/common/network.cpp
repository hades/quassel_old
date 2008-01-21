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
#include "network.h"

#include "signalproxy.h"
#include "ircuser.h"
#include "ircchannel.h"

#include <QDebug>
#include <QTextCodec>

#include "util.h"

// ====================
//  Public:
// ====================
Network::Network(const NetworkId &networkid, QObject *parent) : SyncableObject(parent),
    _networkId(networkid),
    _identity(0),
    _myNick(QString()),
    _networkName(QString("<not initialized>")),
    _currentServer(QString()),
    _connected(false),
    _prefixes(QString()),
    _prefixModes(QString()),
    _proxy(0),
    _codecForEncoding(0),
    _codecForDecoding(0)
{
  setObjectName(QString::number(networkid.toInt()));
}

// I think this is unnecessary since IrcUsers have us as their daddy :)
//Network::~Network() {
//   QHashIterator<QString, IrcUser *> ircuser(_ircUsers);
//   while (ircuser.hasNext()) {
//     ircuser.next();
//     delete ircuser.value();
//   }
//}

NetworkId Network::networkId() const {
  return _networkId;
}

SignalProxy *Network::proxy() const {
  return _proxy;
}

void Network::setProxy(SignalProxy *proxy) {
  _proxy = proxy;
  //proxy->synchronize(this);  // we should to this explicitly from the outside!
}

bool Network::isMyNick(const QString &nick) const {
  return (myNick().toLower() == nick.toLower());
}

bool Network::isMe(IrcUser *ircuser) const {
  return (ircuser->nick().toLower() == myNick().toLower());
}

bool Network::isChannelName(const QString &channelname) const {
  if(channelname.isEmpty())
    return false;
  
  if(supports("CHANTYPES"))
    return support("CHANTYPES").contains(channelname[0]);
  else
    return QString("#&!+").contains(channelname[0]);
}

bool Network::isConnected() const {
  return _connected;
}

QString Network::prefixToMode(const QString &prefix) {
  if(prefixes().contains(prefix))
    return QString(prefixModes()[prefixes().indexOf(prefix)]);
  else
    return QString();
}

QString Network::prefixToMode(const QCharRef &prefix) {
  return prefixToMode(QString(prefix));
}

QString Network::modeToPrefix(const QString &mode) {
  if(prefixModes().contains(mode))
    return QString(prefixes()[prefixModes().indexOf(mode)]);
  else
    return QString();
}

QString Network::modeToPrefix(const QCharRef &mode) {
  return modeToPrefix(QString(mode));
}
  
QString Network::networkName() const {
  return _networkName;
}

QString Network::currentServer() const {
  return _currentServer;
}

QString Network::myNick() const {
  return _myNick;
}

IdentityId Network::identity() const {
  return _identity;
}

QStringList Network::nicks() const {
  // we don't use _ircUsers.keys() since the keys may be
  // not up to date after a nick change
  QStringList nicks;
  foreach(IrcUser *ircuser, _ircUsers.values()) {
    nicks << ircuser->nick();
  }
  return nicks;
}

QStringList Network::channels() const {
  return _ircChannels.keys();
}

QList<QVariantMap> Network::serverList() const {
  return _serverList;
}

QString Network::prefixes() {
  if(_prefixes.isNull())
    determinePrefixes();
  
  return _prefixes;
}

QString Network::prefixModes() {
  if(_prefixModes.isNull())
    determinePrefixes();

  return _prefixModes;
}

bool Network::supports(const QString &param) const {
  return _supports.contains(param);
}

QString Network::support(const QString &param) const {
  QString support_ = param.toUpper();
  if(_supports.contains(support_))
    return _supports[support_];
  else
    return QString();
}

IrcUser *Network::newIrcUser(const QString &hostmask) {
  QString nick(nickFromMask(hostmask).toLower());
  if(!_ircUsers.contains(nick)) {
    IrcUser *ircuser = new IrcUser(hostmask, this);
    // mark IrcUser as already initialized to keep the SignalProxy from requesting initData
    //if(isInitialized())
    //  ircuser->setInitialized();
    if(proxy())
      proxy()->synchronize(ircuser);
    else
      qWarning() << "unable to synchronize new IrcUser" << hostmask << "forgot to call Network::setProxy(SignalProxy *)?";
    
    connect(ircuser, SIGNAL(nickSet(QString)), this, SLOT(ircUserNickChanged(QString)));
    connect(ircuser, SIGNAL(initDone()), this, SLOT(ircUserInitDone()));
    connect(ircuser, SIGNAL(destroyed()), this, SLOT(ircUserDestroyed()));
    _ircUsers[nick] = ircuser;
    emit ircUserAdded(hostmask);
    emit ircUserAdded(ircuser);
  }
  return _ircUsers[nick];
}

IrcUser *Network::newIrcUser(const QByteArray &hostmask) {
  return newIrcUser(decodeString(hostmask));
}

void Network::removeIrcUser(IrcUser *ircuser) {
  QString nick = _ircUsers.key(ircuser);
  if(nick.isNull())
    return;

  _ircUsers.remove(nick);
  emit ircUserRemoved(nick);
  emit ircUserRemoved(ircuser);
  ircuser->deleteLater();
}

void Network::removeIrcUser(QString nick) {
  IrcUser *ircuser;
  if((ircuser = ircUser(nick)) != 0)
    removeIrcUser(ircuser);
}

IrcUser *Network::ircUser(QString nickname) const {
  nickname = nickname.toLower();
  if(_ircUsers.contains(nickname))
    return _ircUsers[nickname];
  else
    return 0;
}

IrcUser *Network::ircUser(const QByteArray &nickname) const {
  return ircUser(decodeString(nickname));
}

QList<IrcUser *> Network::ircUsers() const {
  return _ircUsers.values();
}

quint32 Network::ircUserCount() const {
  return _ircUsers.count();
}

IrcChannel *Network::newIrcChannel(const QString &channelname) {
  if(!_ircChannels.contains(channelname.toLower())) {
    IrcChannel *channel = new IrcChannel(channelname, this);
    // mark IrcUser as already initialized to keep the SignalProxy from requesting initData
    //if(isInitialized())
    //  channel->setInitialized();

    if(proxy())
      proxy()->synchronize(channel);
    else
      qWarning() << "unable to synchronize new IrcChannel" << channelname << "forgot to call Network::setProxy(SignalProxy *)?";

    connect(channel, SIGNAL(initDone()), this, SLOT(ircChannelInitDone()));
    connect(channel, SIGNAL(destroyed()), this, SLOT(channelDestroyed()));
    _ircChannels[channelname.toLower()] = channel;
    emit ircChannelAdded(channelname);
    emit ircChannelAdded(channel);
  }
  return _ircChannels[channelname.toLower()];
}

IrcChannel *Network::newIrcChannel(const QByteArray &channelname) {
  return newIrcChannel(decodeString(channelname));
}

IrcChannel *Network::ircChannel(QString channelname) {
  channelname = channelname.toLower();
  if(_ircChannels.contains(channelname))
    return _ircChannels[channelname];
  else
    return 0;
}

IrcChannel *Network::ircChannel(const QByteArray &channelname) {
  return ircChannel(decodeString(channelname));
}


QList<IrcChannel *> Network::ircChannels() const {
  return _ircChannels.values();
}

quint32 Network::ircChannelCount() const {
  return _ircChannels.count();
}

QByteArray Network::codecForEncoding() const {
  if(_codecForEncoding) return _codecForEncoding->name();
  return QByteArray();
}

void Network::setCodecForEncoding(const QByteArray &name) {
  setCodecForEncoding(QTextCodec::codecForName(name));
}

void Network::setCodecForEncoding(QTextCodec *codec) {
  _codecForEncoding = codec;
}

QByteArray Network::codecForDecoding() const {
  if(_codecForDecoding) return _codecForDecoding->name();
  else return QByteArray();
}

void Network::setCodecForDecoding(const QByteArray &name) {
  setCodecForDecoding(QTextCodec::codecForName(name));
}

void Network::setCodecForDecoding(QTextCodec *codec) {
  _codecForDecoding = codec;
}

QString Network::decodeString(const QByteArray &text) const {
  return ::decodeString(text, _codecForDecoding);
}

QByteArray Network::encodeString(const QString string) const {
  if(_codecForEncoding) {
    return _codecForEncoding->fromUnicode(string);
  }
  return string.toAscii();
}

// ====================
//  Public Slots:
// ====================
void Network::setNetworkName(const QString &networkName) {
  _networkName = networkName;
  emit networkNameSet(networkName);
}

void Network::setCurrentServer(const QString &currentServer) {
  _currentServer = currentServer;
  emit currentServerSet(currentServer);
}

void Network::setConnected(bool connected) {
  _connected = connected;
  emit connectedSet(connected);
}

void Network::setMyNick(const QString &nickname) {
  _myNick = nickname;
  emit myNickSet(nickname);
}

void Network::setIdentity(IdentityId id) {
  _identity = id;
  emit identitySet(id);
}

void Network::setServerList(const QList<QVariantMap> &serverList) {
  _serverList = serverList;
  emit serverListSet(serverList);
}

void Network::addSupport(const QString &param, const QString &value) {
  if(!_supports.contains(param)) {
    _supports[param] = value;
    emit supportAdded(param, value);
  }
}

void Network::removeSupport(const QString &param) {
  if(_supports.contains(param)) {
    _supports.remove(param);
    emit supportRemoved(param);
  }
}

QVariantMap Network::initSupports() const {
  QVariantMap supports;
  QHashIterator<QString, QString> iter(_supports);
  while(iter.hasNext()) {
    iter.next();
    supports[iter.key()] = iter.value();
  }
  return supports;
}

QVariantList Network::initServerList() const {
  QList<QVariant> list;
  foreach(QVariantMap serverdata, serverList()) list << QVariant(serverdata);
  return list;
}

QStringList Network::initIrcUsers() const {
  QStringList hostmasks;
  foreach(IrcUser *ircuser, ircUsers()) {
    hostmasks << ircuser->hostmask();
  }
  return hostmasks;
}

QStringList Network::initIrcChannels() const {
  return _ircChannels.keys();
}

void Network::initSetSupports(const QVariantMap &supports) {
  QMapIterator<QString, QVariant> iter(supports);
  while(iter.hasNext()) {
    iter.next();
    addSupport(iter.key(), iter.value().toString());
  }
}

void Network::initSetServerList(const QVariantList & serverList) {
  QList<QVariantMap> slist;
  foreach(QVariant v, serverList) slist << v.toMap();
  setServerList(slist);
}

void Network::initSetIrcUsers(const QStringList &hostmasks) {
  if(!_ircUsers.empty())
    return;
  foreach(QString hostmask, hostmasks) {
    newIrcUser(hostmask);
  }
}

void Network::initSetChannels(const QStringList &channels) {
  if(!_ircChannels.empty())
    return;
  foreach(QString channel, channels)
    newIrcChannel(channel);
}

IrcUser *Network::updateNickFromMask(const QString &mask) {
  QString nick(nickFromMask(mask).toLower());
  IrcUser *ircuser;
  
  if(_ircUsers.contains(nick)) {
    ircuser = _ircUsers[nick];
    ircuser->updateHostmask(mask);
  } else {
    ircuser = newIrcUser(mask);
  }
  return ircuser;
}

void Network::ircUserNickChanged(QString newnick) {
  QString oldnick = _ircUsers.key(qobject_cast<IrcUser*>(sender()));

  if(oldnick.isNull())
    return;

  if(newnick.toLower() != oldnick) _ircUsers[newnick.toLower()] = _ircUsers.take(oldnick);

  if(myNick().toLower() == oldnick)
    setMyNick(newnick);
}

void Network::ircUserInitDone() {
  IrcUser *ircuser = static_cast<IrcUser *>(sender());
  Q_ASSERT(ircuser);
  emit ircUserInitDone(ircuser);
}

void Network::ircChannelInitDone() {
  IrcChannel *ircchannel = static_cast<IrcChannel *>(sender());
  Q_ASSERT(ircchannel);
  emit ircChannelInitDone(ircchannel);
}

void Network::ircUserDestroyed() {
  IrcUser *ircuser = static_cast<IrcUser *>(sender());
  Q_ASSERT(ircuser);
  removeIrcUser(ircuser);
}

void Network::channelDestroyed() {
  IrcChannel *channel = static_cast<IrcChannel *>(sender());
  Q_ASSERT(channel);
  emit ircChannelRemoved(sender());
  _ircChannels.remove(_ircChannels.key(channel));
}

void Network::requestConnect() {
  if(!proxy()) return;
  if(proxy()->proxyMode() == SignalProxy::Client) emit connectRequested(); // on the client this triggers calling this slot on the core
  else emit connectRequested(networkId());  // and this is for CoreSession :)
}

// ====================
//  Private:
// ====================
void Network::determinePrefixes() {
  // seems like we have to construct them first
  QString PREFIX = support("PREFIX");
  
  if(PREFIX.startsWith("(") && PREFIX.contains(")")) {
    _prefixes = PREFIX.section(")", 1);
    _prefixModes = PREFIX.mid(1).section(")", 0, 0);
  } else {
    QString defaultPrefixes("~&@%+");
    QString defaultPrefixModes("qaohv");

    // we just assume that in PREFIX are only prefix chars stored
    for(int i = 0; i < defaultPrefixes.size(); i++) {
      if(PREFIX.contains(defaultPrefixes[i])) {
	_prefixes += defaultPrefixes[i];
	_prefixModes += defaultPrefixModes[i];
      }
    }
    // check for success
    if(!_prefixes.isNull())
      return;
    
    // well... our assumption was obviously wrong...
    // check if it's only prefix modes
    for(int i = 0; i < defaultPrefixes.size(); i++) {
      if(PREFIX.contains(defaultPrefixModes[i])) {
	_prefixes += defaultPrefixes[i];
	_prefixModes += defaultPrefixModes[i];
      }
    }
    // now we've done all we've could...
  }
}
