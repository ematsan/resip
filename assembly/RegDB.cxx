#include "RegDB.hxx"
#include "rutil/Data.hxx"
#include "rutil/Logger.hxx"

using namespace registrar;
using namespace resip;

#define RESIPROCATE_SUBSYSTEM Subsystem::TEST
/*
static void
encodeString(oDataStream& s, const Data& data)
{
   short len = (short)data.size();
   s.write( (char*)(&len) , sizeof( len ) );
   s.write( data.data(), len );
}


static void
decodeString(iDataStream& s, Data& data)
{
   data.clear();

   if(s.eof()) return;

   short len;
   s.read((char*)(&len), sizeof(len));
   if(s.eof()) return;

   // [TODO] This is probably OK for now, but we can do better than this.
   if (len > 8192)
   {
      ErrLog( << "Tried to decode a database record that was much larger (>8k) than expected.  Returning an empty Data instead." );
      return;
   }

   s.read(data.getBuf(len), len);
}*/
/*************************************************************************/
/*                        USER                                           */
/*************************************************************************/
bool
RegDB::addUser(const Key& key, const UserRecord& rec)
{
  Data command;
  {
     DataStream ds(command);
     ds << "INSERT INTO tuser (fname, fiddomain)"
        << " VALUES('"
        << rec.mName << "', "
        << rec.mIdDomain << ")"
        << " ON DUPLICATE KEY UPDATE"
        << " fname='" << rec.mName
        << "', fiddomain=" << rec.mIdDomain
        << "";
  }
  return query(command, 0) == 0;
}


void
RegDB::eraseUser(const Key& key)
{
  dbEraseRecord(UserTable, key);
}

RegDB::UserRecord
RegDB::getUser(const Key& key) const
{
  UserRecord user;

  return user;
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

// functions for Domain Records
bool
RegDB::addDomain(const Key& key, const DomainRecord& rec)
{
  Data command;
  {
     DataStream ds(command);
     ds << "INSERT INTO tdomain (fdomain)"
        << " VALUES('"
        << rec.mDomain << "')"
        << " ON DUPLICATE KEY UPDATE"
        << " fdomain='" << rec.mDomain
        << "'";
  }
  return query(command, 0) == 0;
}
void
RegDB::eraseDomain(const Key& key)
{
  dbEraseRecord(DomainTable, key);
}

RegDB::DomainRecord
RegDB::getDomain(const Key& key) const
{

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


// functions for Protocol Records
bool
RegDB::addProtocol(const Key& key, const ProtocolRecord& rec)
{
  Data command;
  {
     DataStream ds(command);
     ds << "INSERT INTO tprotocol (fprotocol)"
        << " VALUES('"
        << rec.mProtocol << "')"
        << " ON DUPLICATE KEY UPDATE"
        << " fprotocol='" << rec.mProtocol
        << "'";
  }
  return query(command, 0) == 0;
}

void
RegDB::eraseProtocol(const Key& key)
{
  dbEraseRecord(ProtocolTable, key);
}

RegDB::ProtocolRecord
RegDB::getProtocol(const Key& key) const
{

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

// functions for Forward Records
bool
RegDB::addForward(const Key& key, const ForwardRecord& rec)
{
  Data command;
  {
     DataStream ds(command);
     ds << "INSERT INTO tforward (fidprotocol, faddress, fip, fport)"
        << " VALUES("
        << rec.mIdProtocol << ", '"
        << rec.mAddress << "', '"
        << rec.mIP << "', "
        << rec.mPort << ")"
        << " ON DUPLICATE KEY UPDATE"
        << " fidprotocol=" << rec.mIdProtocol
        << ", faddress='" << rec.mAddress
        << "', fip='" << rec.mIP
        << "', fport=" << rec.mPort
        << "";
  }
  return query(command, 0) == 0;
}

void
RegDB::eraseForward(const Key& key)
{
  dbEraseRecord(ForwardTable, key);
}

RegDB::ForwardRecord
RegDB::getForward(const Key& key) const
{

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

// functions for Authorization Records
bool
RegDB::addAuthorization(const Key& key, const AuthorizationRecord& rec)
{
  Data command;
  {
     DataStream ds(command);
     ds << "INSERT INTO tauthorization (fiduser, fiddomain, fpassword)"
        << " VALUES("
        << rec.mIdUser << ", "
        << rec.mIdDomain << ", '"
        << rec.mPassword << "')"
        << " ON DUPLICATE KEY UPDATE"
        << " fiduser=" << rec.mIdUser
        << ", fiddomain=" << rec.mIdDomain
        << ", fpassword='" << rec.mPassword
        << "'";
  }
  return query(command, 0) == 0;
}

void
RegDB::eraseAuthorization(const Key& key)
{
    dbEraseRecord(AuthorizationTable, key);
}

RegDB::AuthorizationRecord
RegDB::getAuthorization(const Key& key) const
{

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

// functions for Registrar Records
bool
RegDB::addRegistrar(const Key& key, const RegistrarRecord& rec)
{
  Data command;
  {
     DataStream ds(command);
     ds << "INSERT INTO tregistrar (fiduser, fiddomain, fidmain)"
        << " VALUES("
        << rec.mIdUser << ", "
        << rec.mIdDomain << ", "
        << rec.mIdMain << ")"
        << " ON DUPLICATE KEY UPDATE"
        << " fiduser=" << rec.mIdUser
        << ", fiddomain=" << rec.mIdDomain
        << ", fidmain=" << rec.mIdMain
        << "";
  }
  return query(command, 0) == 0;
}

void
RegDB::eraseRegistrar(const Key& key)
{
  dbEraseRecord(RegistrarTable, key);
}

RegDB::RegistrarRecord
RegDB::getRegistrar(const Key& key) const
{

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

// functions for Route Records
bool
RegDB::addRoute(const Key& key, const RouteRecord& rec)
{
  Data command;
  {
     DataStream ds(command);
     ds << "INSERT INTO troute (fidreg, fidforward, ftime, fexpires)"
        << " VALUES("
        << rec.mIdReg << ", "
        << rec.mIdForward << ", '"
        << rec.mTime << "', "
        << rec.mExpires << ")"
        << " ON DUPLICATE KEY UPDATE"
        << " fidreg=" << rec.mIdReg
        << ", fidforward=" << rec.mIdForward
        << ", ftime='" << rec.mTime
        << "', fexpires=" << rec.mExpires
        << "";
  }
  return query(command, 0) == 0;
}

void
RegDB::eraseRoute(const Key& key)
{
  dbEraseRecord(RouteTable, key);
}

RegDB::RouteRecord
RegDB::getRoute(const Key& key) const
{

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
     ds << " WHERE id='" << key << "'";
  }
  query(command);
}
