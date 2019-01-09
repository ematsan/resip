#include "RegDB.hxx"
#include "rutil/Data.hxx"
#include "rutil/Logger.hxx"

using namespace registrar;
using namespace resip;

/*************************************************************************/
/*                        USER                                           */
/*************************************************************************/
bool
RegDB::addUser(const UserRecord& rec)
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
RegDB::eraseUser(const Key& key)
{
  dbEraseRecord(UserTable, key);
}

RegDB::UserRecord
RegDB::getUser(const Key& key) const
{
  UserRecord rec;
  Data command;
  {
     DataStream ds(command);
     ds << "SELECT fname FROM tuser"
        << " WHERE fiduser='" << key
        << "'";
  }
  if(query(command, rec, key) != 0)
  {
     return rec;
  }
  if (rec.mIdUser == 0)
  {
     ErrLog( << "Base store result failed");
     return rec;
  }
  return rec;
}

RegDB::UserRecordList
RegDB::getAllUsers()
{
  UserRecordList records;
  Key key = dbKey(UserTable, true);
  while ( !key.empty() )
  {
     UserRecord rec = getUser(key);
     records.push_back(rec);
     key = dbKey(UserTable, false);
  }
  return records;
}

int
RegDB::findUser(UserRecord& rec)
{
  Data command;
  {
     DataStream ds(command);
     ds << "SELECT fiduser FROM tuser"
        << " WHERE fname='" << rec.mName
        << "'";
  }
  if(query(command, rec, 0) != 0)
  {
     return -1;
  }
  if (rec.mIdUser == 0)
  {
     ErrLog( << "Base store result failed");
     return 0;
  }
  return rec.mIdUser;
}
/*************************************************************************/
/*                       DOMAIN                                          */
/*************************************************************************/
bool
RegDB::addDomain(const DomainRecord& rec)
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
RegDB::eraseDomain(const Key& key)
{
  dbEraseRecord(DomainTable, key);
}

RegDB::DomainRecord
RegDB::getDomain(const Key& key) const
{
  DomainRecord rec;
  Data command;
  {
     DataStream ds(command);
     ds << "SELECT fdomain, fidrealm FROM tdomain"
        << " WHERE fiddomain='" << key
        << "'";
  }
  if(query(command, rec, key) != 0)
  {
     return rec;
  }
  if (rec.mIdDomain == 0)
  {
     ErrLog( << "Base store result failed");
     return rec;
  }
  return rec;
}

RegDB::DomainRecordList
RegDB::getAllDomains()
{
  DomainRecordList records;
  Key key = dbKey(DomainTable, true);
  while ( !key.empty() )
  {
     DomainRecord rec = getDomain(key);
     records.push_back(rec);
     key = dbKey(DomainTable, false);
  }
  return records;
}


int
RegDB::findDomain(DomainRecord& rec)
{
  Data command;
  {
     DataStream ds(command);
     ds << "SELECT fiddomain FROM tdomain"
        << " WHERE fdomain='" << rec.mDomain
        << "'";
  }
  if(query(command, rec, 0) != 0)
  {
     return -1;
  }
  if (rec.mIdDomain == 0)
  {
     ErrLog( << "Base store result failed");
     return 0;
  }
  return rec.mIdDomain;
}
/*************************************************************************/
/*                       USER DOMAIN                                     */
/*************************************************************************/
bool
RegDB::addUserDomain(const UserDomainRecord& rec)
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
RegDB::eraseUserDomain(const Key& key)
{
  dbEraseRecord(UserDomainTable, key);
}

RegDB::UserDomainRecord
RegDB::getUserDomain(const Key& key) const
{
  UserDomainRecord rec;
  Data command;
  {
     DataStream ds(command);
     ds << "SELECT fiddomainfk, fiduserfk FROM tuserdomain"
        << " WHERE fidud='" << key
        << "'";
  }
  if(query(command, rec, key) != 0)
  {
     return rec;
  }
  if (rec.mIdUserDomain == 0)
  {
     ErrLog( << "Base store result failed");
     return rec;
  }
  return rec;
}

RegDB::UserDomainRecordList
RegDB::getAllUserDomains()
{
  UserDomainRecordList records;
  Key key = dbKey(UserDomainTable, true);
  while ( !key.empty() )
  {
     UserDomainRecord rec = getUserDomain(key);
     records.push_back(rec);
     key = dbKey(UserDomainTable, false);
  }
  return records;
}


int
RegDB::findUserDomain(UserDomainRecord& rec)
{
  Data command;
  {
     DataStream ds(command);
     ds << "SELECT fidud FROM tuserdomain"
        << " WHERE fiddomainfk = '" << rec.mIdDomainFk
        << "' and fiduserfk = '" << rec.mIdUserFk
        << "'";
  }
  if(query(command, rec, 0) != 0)
  {
     return -1;
  }
  if (rec.mIdUserDomain == 0)
  {
     ErrLog( << "Base store result failed");
     return 0;
  }
  return rec.mIdUserDomain;
}
/*************************************************************************/
/*                        PROTOCOL                                       */
/*************************************************************************/
bool
RegDB::addProtocol(const ProtocolRecord& rec)
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
RegDB::eraseProtocol(const Key& key)
{
  dbEraseRecord(ProtocolTable, key);
}

RegDB::ProtocolRecord
RegDB::getProtocol(const Key& key) const
{
  ProtocolRecord rec;
  Data command;
  {
     DataStream ds(command);
     ds << "SELECT fprotocol FROM tprotocol"
        << " WHERE fidprotocol='" << key
        << "'";
  }
  if(query(command, rec, key) != 0)
  {
     return rec;
  }
  if (rec.mIdProtocol == 0)
  {
     ErrLog( << "Base store result failed");
     return rec;
  }
  return rec;
}

RegDB::ProtocolRecordList
RegDB::getAllProtocols()
{
  ProtocolRecordList records;
  Key key = dbKey(ProtocolTable, true);
  while ( !key.empty() )
  {
     ProtocolRecord rec = getProtocol(key);
     records.push_back(rec);
     key = dbKey(ProtocolTable, false);
  }
  return records;
}

int
RegDB::findProtocol(ProtocolRecord& rec)
{
  Data command;
  {
     DataStream ds(command);
     ds << "SELECT fidprotocol FROM tprotocol"
        << " WHERE fprotocol = '" << rec.mProtocol
        << "'";
  }
  if(query(command, rec, 0) != 0)
  {
     return -1;
  }
  if (rec.mIdProtocol == 0)
  {
     ErrLog( << "Base store result failed");
     return 0;
  }
  return rec.mIdProtocol;
}
/*************************************************************************/
/*                        FORWARD                                        */
/*************************************************************************/
bool
RegDB::addForward(const ForwardRecord& rec)
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
RegDB::eraseForward(const Key& key)
{
  dbEraseRecord(ForwardTable, key);
}

RegDB::ForwardRecord
RegDB::getForward(const Key& key) const
{
  ForwardRecord rec;
  Data command;
  {
     DataStream ds(command);
     ds << "SELECT fidprotocolfk, fiddomainfk, fport FROM tforward"
        << " WHERE fidforward='" << key
        << "'";
  }
  if(query(command, rec, key) != 0)
  {
     return rec;
  }
  if (rec.mIdForward == 0)
  {
     ErrLog( << "Base store result failed");
     return rec;
  }
  return rec;
}

RegDB::ForwardRecordList
RegDB::getAllForwards()
{
  ForwardRecordList records;
  Key key = dbKey(ForwardTable, true);
  while ( !key.empty() )
  {
     ForwardRecord rec = getForward(key);
     records.push_back(rec);
     key = dbKey(ForwardTable, false);
  }
  return records;
}

int
RegDB::findForward(ForwardRecord& rec)
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
  if(query(command, rec, 0) != 0)
  {
     return -1;
  }
  if (rec.mIdForward == 0)
  {
     ErrLog( << "Base store result failed");
     return 0;
  }
  return rec.mIdForward;
}
/*************************************************************************/
/*                        AUTHORIZATION                                  */
/*************************************************************************/
bool
RegDB::addAuthorization(const AuthorizationRecord& rec)
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
RegDB::eraseAuthorization(const Key& key)
{
    dbEraseRecord(AuthorizationTable, key);
}

RegDB::AuthorizationRecord
RegDB::getAuthorization(const Key& key) const
{
  AuthorizationRecord rec;
  Data command;
  {
     DataStream ds(command);
     ds << "SELECT fidudfk, fpassword FROM tauthorization"
        << " WHERE fidauth='" << key
        << "'";
  }
  if(query(command, rec, key) != 0)
  {
     return rec;
  }
  if (rec.mIdAuth == 0)
  {
     ErrLog( << "Base store result failed");
     return rec;
  }
  return rec;
}

RegDB::AuthorizationRecordList
RegDB::getAllAuthorizations()
{
  AuthorizationRecordList records;
  Key key = dbKey(AuthorizationTable, true);
  while ( !key.empty() )
  {
     AuthorizationRecord rec = getAuthorization(key);
     records.push_back(rec);
     key = dbKey(AuthorizationTable, false);
  }
  return records;
}


int
RegDB::findAuthorization(AuthorizationRecord& rec)
{
  Data command;
  {
     DataStream ds(command);
     ds << "SELECT fidauth FROM tauthorization"
        << " WHERE fidudfk = '" << rec.mIdUserDomainFk
        << "' and fpassword = '" << rec.mPassword
        << "'";
  }
  if(query(command, rec, 0) != 0)
  {
     return -1;
  }
  if (rec.mIdAuth == 0)
  {
     ErrLog( << "Base store result failed");
     return 0;
  }
  return rec.mIdAuth;
}
/*************************************************************************/
/*                        REGISTRAR                                      */
/*************************************************************************/
bool
RegDB::addRegistrar(const RegistrarRecord& rec)
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
RegDB::eraseRegistrar(const Key& key)
{
  dbEraseRecord(RegistrarTable, key);
}

RegDB::RegistrarRecord
RegDB::getRegistrar(const Key& key) const
{
  RegistrarRecord rec;
  Data command;
  {
     DataStream ds(command);
     ds << "SELECT fidudfk, fcallid, fidmainfk FROM tregistrar"
        << " WHERE fidreg='" << key
        << "'";
  }
  if(query(command, rec, key) != 0)
  {
     return rec;
  }
  if (rec.mIdReg == 0)
  {
     ErrLog( << "Base store result failed");
     return rec;
  }
  return rec;
}

RegDB::RegistrarRecordList
RegDB::getAllRegistrars()
{
  RegistrarRecordList records;
  Key key = dbKey(RegistrarTable, true);
  while ( !key.empty() )
  {
     RegistrarRecord rec = getRegistrar(key);
     records.push_back(rec);
     key = dbKey(RegistrarTable, false);
  }
  return records;
}

bool
RegDB::updateRegistrar(const Key& key, const RegistrarRecord& rec)
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
RegDB::findRegistrar(RegistrarRecord& rec)
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

  if(query(command, rec, 0) != 0)
  {
     return -1;
  }
  if (rec.mIdReg == 0)
  {
     ErrLog( << "Base store result failed");
     return 0;
  }
  return rec.mIdReg;
  }
/*************************************************************************/
/*                        ROUTE                                          */
/*************************************************************************/
bool
RegDB::addRoute(const RouteRecord& rec)
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
RegDB::eraseRoute(const Key& key)
{
  dbEraseRecord(RouteTable, key);
}

RegDB::RouteRecord
RegDB::getRoute(const Key& key) const
{
  RouteRecord rec;
  Data command;
  {
     DataStream ds(command);
     ds << "SELECT fidregfk, fidforwardfk, ftime, fexpires FROM troute"
        << " WHERE fidroute='" << key
        << "'";
  }

  if(query(command, rec, key) != 0)
  {
     return rec;
  }
  if (rec.mIdRoute == 0)
  {
     ErrLog( << "Base store result failed");
     return rec;
  }
  return rec;
}

RegDB::RouteRecordList
RegDB::getAllRoutes()
{
  RouteRecordList records;
  Key key = dbKey(RouteTable, true);
  while ( !key.empty() )
  {
     RouteRecord rec = getRoute(key);
     records.push_back(rec);
     key = dbKey(RouteTable, false);
  }
  return records;
}

bool
RegDB::updateRoute(const Key& key, const RouteRecord& rec)
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
RegDB::findRoute(RouteRecord& rec)
{
  Data command;
  {
     DataStream ds(command);
     ds << "SELECT fidroute FROM troute"
        << " WHERE fidregfk = '" << rec.mIdRegFk
        << "' and fidforwardfk = '" << rec.mIdForwardFk
        << "' and ftime = '" << rec.mTime
        << "'";
  }

  if(query(command, rec, 0) != 0)
  {
     return -1;
  }
  if (rec.mIdRoute == 0)
  {
     ErrLog( << "Base store result failed");
     return 0;
  }
  return rec.mIdRoute;
}
/*****************************************************************************/
void
RegDB::dbEraseRecord(const Table table,
                        const Data& key)
{
  Data command;
  {
     DataStream ds(command);
     Data escapedKey;
     ds << "DELETE FROM " << tableName[table];
     ds << " WHERE "<< keyName[table]<<" = '" << key << "'";
  }
  query(command);
}
