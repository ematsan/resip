#include "RegDB.hxx"
#include "rutil/Data.hxx"
#include "rutil/Logger.hxx"

using namespace registrar;
using namespace resip;

/*************************************************************************/
/*                        USER                                           */
/*************************************************************************/
bool
RegDB::addUser(const UserRecord& rec) const
{
  Data command;
  {
     DataStream ds(command);
     ds << "INSERT INTO tuser (fname)"
        << " VALUES('"
        << rec.mName << "')";
  }
  return query(command) == 0;
}

void
RegDB::eraseUser(const Key& key) const
{
  Data command;
  {
     DataStream ds(command);
     Data escapedKey;
     ds << "DELETE FROM tuser";
     ds << " WHERE fiduser = '" << key << "'";
  }
  query(command);
}

RegDB::UserRecord
RegDB::getUser(const Key& key) const
{
  UserRecord rec;
  Data command;
  {
     DataStream ds(command);
     ds << "SELECT fiduser, fname FROM tuser"
        << " WHERE fiduser='" << key
        << "'";
  }

  if(query(command, rec) != 0)
  {
     return rec;
  }
  if (rec.mIdUser == 0)
  {
     ErrLog( << "getUser result failed");
     return rec;
  }
  return rec;
}

int
RegDB::findUserId(UserRecord& rec) const
{
  Data command;
  {
     DataStream ds(command);
     ds << "SELECT fiduser FROM tuser"
        << " WHERE fname='" << rec.mName
        << "'";
  }
  if(query(command, rec) != 0)
  {
     return -1;
  }
  if (rec.mIdUser == 0)
  {
     ErrLog( << "findUserId result failed");
     return 0;
  }
  return rec.mIdUser;
}
/*************************************************************************/
/*                       DOMAIN                                          */
/*************************************************************************/
bool
RegDB::addDomain(const DomainRecord& rec) const
{
  Data command;
  {
     DataStream ds(command);
     ds << "INSERT INTO tdomain (fdomain, fidrealm)"
        << " VALUES('"
        << rec.mDomain << "', "
        << rec.mIdRealm<<")";
  }
  return query(command) == 0;
}

void
RegDB::eraseDomain(const Key& key) const
{
  Data command;
  {
     DataStream ds(command);
     Data escapedKey;
     ds << "DELETE FROM tdomain";
     ds << " WHERE fiddomain = '" << key << "'";
  }
  query(command);
}

RegDB::DomainRecord
RegDB::getDomain(const Key& key) const
{
  DomainRecord rec;
  Data command;
  {
     DataStream ds(command);
     ds << "SELECT fiddomain, fdomain, fidrealm FROM tdomain"
        << " WHERE fiddomain='" << key
        << "'";
  }
  if(query(command, rec) != 0)
  {
     return rec;
  }
  if (rec.mIdDomain == 0)
  {
     ErrLog( << "getDomain result failed");
     return rec;
  }
  return rec;
}

int
RegDB::findDomainId(DomainRecord& rec) const
{
  Data command;
  {
     DataStream ds(command);
     ds << "SELECT fiddomain FROM tdomain"
        << " WHERE fdomain='" << rec.mDomain
        << "'";
  }
  if(query(command, rec) != 0)
  {
     return -1;
  }
  if (rec.mIdDomain == 0)
  {
     ErrLog( << "findDomainId result failed");
     return 0;
  }
  return rec.mIdDomain;
}
/*************************************************************************/
/*                       USER DOMAIN                                     */
/*************************************************************************/
bool
RegDB::addUserDomain(const UserDomainRecord& rec) const
{
  Data command;
  {
     DataStream ds(command);
     ds << "INSERT INTO tuserdomain (fiduserfk, fiddomainfk)"
        << " VALUES("
        << rec.mIdUserFk << ","
        << rec.mIdDomainFk<<")";
  }
  return query(command) == 0;
}

void
RegDB::eraseUserDomain(const Key& key) const
{
  Data command;
  {
     DataStream ds(command);
     Data escapedKey;
     ds << "DELETE FROM tuserdomain";
     ds << " WHERE fidud = '" << key << "'";
  }
  query(command);
}

RegDB::UserDomainRecord
RegDB::getUserDomain(const Key& key) const
{
  UserDomainRecord rec;
  Data command;
  {
     DataStream ds(command);
     ds << "SELECT fidud, fiddomainfk, fiduserfk FROM tuserdomain"
        << " WHERE fidud='" << key
        << "'";
  }
  if(query(command, rec) != 0)
  {
     return rec;
  }
  if (rec.mIdUserDomain == 0)
  {
     ErrLog( << "getUserDomain result failed");
     return rec;
  }
  return rec;
}

int
RegDB::findUserDomainId(UserDomainRecord& rec) const
{
  Data command;
  {
     DataStream ds(command);
     ds << "SELECT fidud FROM tuserdomain"
        << " WHERE fiddomainfk = '" << rec.mIdDomainFk
        << "' and fiduserfk = '" << rec.mIdUserFk
        << "'";
  }
  if(query(command, rec) != 0)
  {
     return -1;
  }
  if (rec.mIdUserDomain == 0)
  {
     ErrLog( << "findUserDomainId result failed");
     return 0;
  }
  return rec.mIdUserDomain;
}
/*************************************************************************/
/*                        PROTOCOL                                       */
/*************************************************************************/
bool
RegDB::addProtocol(const ProtocolRecord& rec) const
{
  Data command;
  {
     DataStream ds(command);
     ds << "INSERT INTO tprotocol (fprotocol)"
        << " VALUES('"
        << rec.mProtocol << "')";
    }
  return query(command) == 0;
}

void
RegDB::eraseProtocol(const Key& key) const
{
  Data command;
  {
     DataStream ds(command);
     Data escapedKey;
     ds << "DELETE FROM tprotocol";
     ds << " WHERE fidprotocol = '" << key << "'";
  }
  query(command);
}

RegDB::ProtocolRecord
RegDB::getProtocol(const Key& key) const
{
  ProtocolRecord rec;
  Data command;
  {
     DataStream ds(command);
     ds << "SELECT fidprotocol, fprotocol FROM tprotocol"
        << " WHERE fidprotocol='" << key
        << "'";
  }
  if(query(command, rec) != 0)
  {
     return rec;
  }
  if (rec.mIdProtocol == 0)
  {
     ErrLog( << "getProtocol result failed");
     return rec;
  }
  return rec;
}


int
RegDB::findProtocolId(ProtocolRecord& rec) const
{
  Data command;
  {
     DataStream ds(command);
     ds << "SELECT fidprotocol FROM tprotocol"
        << " WHERE fprotocol = '" << rec.mProtocol
        << "'";
  }
  if(query(command, rec) != 0)
  {
     return -1;
  }
  if (rec.mIdProtocol == 0)
  {
     ErrLog( << "findProtocolId result failed");
     return 0;
  }
  return rec.mIdProtocol;
}
/*************************************************************************/
/*                        FORWARD                                        */
/*************************************************************************/
bool
RegDB::addForward(const ForwardRecord& rec) const
{
  Data command;
  {
     DataStream ds(command);
     ds << "INSERT INTO tforward (fidprotocolfk, fiddomainfk, fport)"
        << " VALUES("
        << rec.mIdProtocolFk << ", "
        << rec.mIdDomainFk << ", "
        << rec.mPort << ")";
  }
  return query(command) == 0;
}

void
RegDB::eraseForward(const Key& key) const
{
  Data command;
  {
     DataStream ds(command);
     Data escapedKey;
     ds << "DELETE FROM tforward";
     ds << " WHERE fidforward = '" << key << "'";
  }
  query(command);
}

RegDB::ForwardRecord
RegDB::getForward(const Key& key) const
{
  ForwardRecord rec;
  Data command;
  {
     DataStream ds(command);
     ds << "SELECT fidforward, fidprotocolfk, fiddomainfk, fport FROM tforward"
        << " WHERE fidforward='" << key
        << "'";
  }
  if(query(command, rec) != 0)
  {
     return rec;
  }
  if (rec.mIdForward == 0)
  {
     ErrLog( << "getForward result failed");
     return rec;
  }
  return rec;
}

int
RegDB::findForwardId(ForwardRecord& rec) const
{
  Data command;
  {
     DataStream ds(command);
     ds << "SELECT fidforward FROM tforward"
        << " WHERE fidforwardfk = '" << rec.mIdDomainFk
        << "' and fidprotocol = '" << rec.mIdProtocolFk
        << "' and fport = '" << rec.mPort
        << "'";
  }
  if(query(command, rec) != 0)
  {
     return -1;
  }
  if (rec.mIdForward == 0)
  {
     ErrLog( << "findForwardId result failed");
     return 0;
  }
  return rec.mIdForward;
}
/*************************************************************************/
/*                        AUTHORIZATION                                  */
/*************************************************************************/
bool
RegDB::addAuthorization(const AuthorizationRecord& rec) const
{
  Data command;
  {
     DataStream ds(command);
     ds << "INSERT INTO tauthorization (fidudfk, fpassword)"
        << " VALUES("
        << rec.mIdUserDomainFk << ", '"
        << rec.mPassword << "')";
  }
  return query(command) == 0;
}

void
RegDB::eraseAuthorization(const Key& key) const
{
  Data command;
  {
     DataStream ds(command);
     Data escapedKey;
     ds << "DELETE FROM tauthorization";
     ds << " WHERE fidauth = '" << key << "'";
  }
  query(command);
}

RegDB::AuthorizationRecord
RegDB::getAuthorization(const Key& key) const
{
  AuthorizationRecord rec;
  Data command;
  {
     DataStream ds(command);
     ds << "SELECT fidauth, fidudfk, fpassword FROM tauthorization"
        << " WHERE fidauth='" << key
        << "'";
  }
  if(query(command, rec) != 0)
  {
     return rec;
  }
  if (rec.mIdAuth == 0)
  {
     ErrLog( << "getAuthorization result failed");
     return rec;
  }
  return rec;
}

int
RegDB::findAuthorizationId(AuthorizationRecord& rec) const
{
  Data command;
  {
     DataStream ds(command);
     ds << "SELECT fidauth FROM tauthorization"
        << " WHERE fidudfk = '" << rec.mIdUserDomainFk
        << "' and fpassword = '" << rec.mPassword
        << "'";
  }
  if(query(command, rec) != 0)
  {
     return -1;
  }
  if (rec.mIdAuth == 0)
  {
     ErrLog( << "findAuthorizationId result failed");
     return 0;
  }
  return rec.mIdAuth;
}
/*************************************************************************/
/*                        REGISTRAR                                      */
/*************************************************************************/
bool
RegDB::addRegistrar(const RegistrarRecord& rec) const
{
  Data command;
  {
     DataStream ds(command);
     ds << "INSERT INTO tregistrar (fidudfk, fcallid, fidmainfk)"
        << " VALUES("
        << rec.mIdUserDomainFk << ", '"
        << rec.mCallId<<"', "
        << rec.mIdMainFk << ")";
  }
  return query(command) == 0;
}

void
RegDB::eraseRegistrar(const Key& key) const
{
  Data command;
  {
     DataStream ds(command);
     Data escapedKey;
     ds << "DELETE FROM tregistrar";
     ds << " WHERE fidreg = '" << key << "'";
  }
  query(command);
}

RegDB::RegistrarRecord
RegDB::getRegistrar(const Key& key) const
{
  RegistrarRecord rec;
  Data command;
  {
     DataStream ds(command);
     ds << "SELECT fidreg, fidudfk, fcallid, fidmainfk FROM tregistrar"
        << " WHERE fidreg='" << key
        << "'";
  }
  if(query(command, rec) != 0)
  {
     return rec;
  }
  if (rec.mIdReg == 0)
  {
     ErrLog( << "getRegistrar result failed");
     return rec;
  }
  return rec;
}

bool
RegDB::updateRegistrar(const Key& key, const RegistrarRecord& rec) const
{
  Data command;
  {
     DataStream ds(command);
     ds << "UPDATE tregistrar SET"
        << " fcallid = '" << rec.mCallId
        << "' WHERE fidreg = " << key;
  }
  return query(command) == 0;
}

int
RegDB::findRegistrarId(RegistrarRecord& rec) const
{
  Data command;
  {
     DataStream ds(command);
     ds << "SELECT fidreg FROM tregistrar"
        << " WHERE fidudfk = '" << rec.mIdUserDomainFk
        << "' and fidmainfk = '" << rec.mIdMainFk
        << "' and fcallid = '" << rec.mCallId
        << "'";
  }

  if(query(command, rec) != 0)
  {
     return -1;
  }
  if (rec.mIdReg == 0)
  {
     ErrLog( << "findRegistrarId result failed");
     return 0;
  }
  return rec.mIdReg;
  }
/*************************************************************************/
/*                        ROUTE                                          */
/*************************************************************************/
bool
RegDB::addRoute(const RouteRecord& rec) const
{
  Data command;
  {
     DataStream ds(command);
     ds << "INSERT INTO troute (fidregfk, fidforwardfk, ftime, fexpires)"
        << " VALUES("
        << rec.mIdRegFk << ", "
        << rec.mIdForwardFk << ", '"
        << rec.mTime << "', "
        << rec.mExpires << ")";
  }
  return query(command) == 0;
}

void
RegDB::eraseRoute(const Key& key) const
{
  Data command;
  {
     DataStream ds(command);
     Data escapedKey;
     ds << "DELETE FROM troute";
     ds << " WHERE fidroute = '" << key << "'";
  }
  query(command);
}

RegDB::RouteRecord
RegDB::getRoute(const Key& key) const
{
  RouteRecord rec;
  Data command;
  {
     DataStream ds(command);
     ds << "SELECT fidroute, fidregfk, fidforwardfk, ftime, fexpires FROM troute"
        << " WHERE fidroute='" << key
        << "'";
  }

  if(query(command, rec) != 0)
  {
     return rec;
  }
  if (rec.mIdRoute == 0)
  {
     ErrLog( << "getRoute result failed");
     return rec;
  }
  return rec;
}

bool
RegDB::updateRoute(const Key& key, const RouteRecord& rec) const
{
  Data command;
  {
     DataStream ds(command);
     ds << "UPDATE troute SET"
        << " fidregfk = " << rec.mIdRegFk
        << ", fidforwardfk = " << rec.mIdForwardFk
        << ", ftime = '" << rec.mTime
        << "', fexpires = " << rec.mExpires
        << " WHERE fidroute = " << key;
  }
  return query(command) == 0;
}

int
RegDB::findRouteId(RouteRecord& rec) const
{
  Data command;
  {
     DataStream ds(command);
     ds << "SELECT fidroute FROM troute"
        << " WHERE fidregfk = '" << rec.mIdRegFk
        << "' and fidforwardfk = '" << rec.mIdForwardFk
      //  << "' and ftime = '" << rec.mTime
        << "'";
  }

  if(query(command, rec) != 0)
  {
     return -1;
  }
  if (rec.mIdRoute == 0)
  {
     ErrLog( << "findRouteId result failed");
     return 0;
  }
  return rec.mIdRoute;
}
