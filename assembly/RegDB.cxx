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
        << rec.mIdDomain << ")";
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
  UserRecord rec;
  Data command;
  {
     DataStream ds(command);
     ds << "SELECT fname, fiddomain FROM tuser"
        << " WHERE fiduser='" << key
        << "'";
  }

  MYSQL_RES* result = 0;
  if(query(command, &result) != 0)
  {
     return rec;
  }
  if (result == 0)
  {
     ErrLog( << "MySQL store result failed: error=" << mysql_errno(mConn) << ": " << mysql_error(mConn));
     return rec;
  }

  MYSQL_ROW row=mysql_fetch_row(result);
  if(row)
   {
       int col = 0;
       rec.mIdUser          = Data(key).convertInt();
       rec.mName            = Data(row[col++]);
       rec.mIdDomain        = Data(row[col++]).convertInt();
   }
  mysql_free_result(result);
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
/*************************************************************************/
/*                       DOMAIN                                          */
/*************************************************************************/
bool
RegDB::addDomain(const Key& key, const DomainRecord& rec)
{
  Data command;
  {
     DataStream ds(command);
     ds << "INSERT INTO tdomain (fdomain, fidrealm)"
        << " VALUES('"
        << rec.mDomain << ","
        << rec.mIdRealm<<"')";
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
  DomainRecord rec;
  Data command;
  {
     DataStream ds(command);
     ds << "SELECT fdomain, fidrealm FROM tdomain"
        << " WHERE fiddomain='" << key
        << "'";
  }

  MYSQL_RES* result = 0;
  if(query(command, &result) != 0)
  {
     return rec;
  }
  if (result == 0)
  {
     ErrLog( << "MySQL store result failed: error=" << mysql_errno(mConn) << ": " << mysql_error(mConn));
     return rec;
  }

  MYSQL_ROW row=mysql_fetch_row(result);
  if(row)
   {
       int col = 0;
       rec.mIdDomain          = Data(key).convertInt();
       rec.mDomain          = Data(row[col++]);
       rec.mIdRealm          = Data(row[col++]).convertInt();
   }
  mysql_free_result(result);
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
/*************************************************************************/
/*                       REALM                                         */
/*************************************************************************/
bool
RegDB::addRealm(const Key& key, const RealmRecord& rec)
{
  Data command;
  {
     DataStream ds(command);
     ds << "INSERT INTO trealm (frealm)"
        << " VALUES('"
        << rec.mRealm << "')";
  }
  return query(command, 0) == 0;
}
void
RegDB::eraseRealm(const Key& key)
{
  dbEraseRecord(RealmTable, key);
}

RegDB::RealmRecord
RegDB::getRealm(const Key& key) const
{
  RealmRecord rec;
  Data command;
  {
     DataStream ds(command);
     ds << "SELECT frealm FROM trealm"
        << " WHERE fidrealm='" << key
        << "'";
  }

  MYSQL_RES* result = 0;
  if(query(command, &result) != 0)
  {
     return rec;
  }
  if (result == 0)
  {
     ErrLog( << "MySQL store result failed: error=" << mysql_errno(mConn) << ": " << mysql_error(mConn));
     return rec;
  }

  MYSQL_ROW row=mysql_fetch_row(result);
  if(row)
   {
       int col = 0;
       rec.mIdRealm = Data(key).convertInt();
       rec.mRealm          = Data(row[col++]);
   }
  mysql_free_result(result);
  return rec;
}

RegDB::RealmRecordList
RegDB::getAllRealms()
{
  RealmRecordList records;
  Key key = dbKey(RealmTable, true);
  while ( !key.empty() )
  {
     RealmRecord rec = getRealm(key);
     records.push_back(rec);
     key = dbKey(RealmTable, false);
  }
  return records;
}
/*************************************************************************/
/*                        PROTOCOL                                       */
/*************************************************************************/
bool
RegDB::addProtocol(const Key& key, const ProtocolRecord& rec)
{
  Data command;
  {
     DataStream ds(command);
     ds << "INSERT INTO tprotocol (fprotocol)"
        << " VALUES('"
        << rec.mProtocol << "')";
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
  ProtocolRecord rec;
  Data command;
  {
     DataStream ds(command);
     ds << "SELECT fprotocol FROM tprotocol"
        << " WHERE fidprotocol='" << key
        << "'";
  }

  MYSQL_RES* result = 0;
  if(query(command, &result) != 0)
  {
     return rec;
  }
  if (result == 0)
  {
     ErrLog( << "MySQL store result failed: error=" << mysql_errno(mConn) << ": " << mysql_error(mConn));
     return rec;
  }

  MYSQL_ROW row=mysql_fetch_row(result);
  if(row)
   {
       int col = 0;
       rec.mIdProtocol          = Data(key).convertInt();
       rec.mProtocol            = Data(row[col++]);
   }
  mysql_free_result(result);
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
/*************************************************************************/
/*                        FORWARD                                        */
/*************************************************************************/
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
        << rec.mPort << ")";
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
  ForwardRecord rec;
  Data command;
  {
     DataStream ds(command);
     ds << "SELECT fidprotocol, faddres, fip, fport FROM tforward"
        << " WHERE fidforward='" << key
        << "'";
  }

  MYSQL_RES* result = 0;
  if(query(command, &result) != 0)
  {
     return rec;
  }
  if (result == 0)
  {
     ErrLog( << "MySQL store result failed: error=" << mysql_errno(mConn) << ": " << mysql_error(mConn));
     return rec;
  }

  MYSQL_ROW row=mysql_fetch_row(result);
  if(row)
   {
       int col = 0;
       rec.mIdForward            = Data(key).convertInt();
       rec.mIdProtocol            = Data(row[col++]).convertInt();
       rec.mAddress               = Data(row[col++]);
       rec.mIP                    = Data(row[col++]);
       rec.mPort                  = Data(row[col++]).convertInt();
   }
  mysql_free_result(result);
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
/*************************************************************************/
/*                        AUTHORIZATION                                  */
/*************************************************************************/
bool
RegDB::addAuthorization(const Key& key, const AuthorizationRecord& rec)
{
  Data command;
  {
     DataStream ds(command);
     ds << "INSERT INTO tauthorization (fiduser, fidrealm, fpassword)"
        << " VALUES("
        << rec.mIdUser << ", "
        << rec.mIdRealm << ", '"
        << rec.mPassword << "')";
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
  AuthorizationRecord rec;
  Data command;
  {
     DataStream ds(command);
     ds << "SELECT fiduser, fidrealm, fpassword FROM tauthorization"
        << " WHERE fidauth='" << key
        << "'";
  }

  MYSQL_RES* result = 0;
  if(query(command, &result) != 0)
  {
     return rec;
  }
  if (result == 0)
  {
     ErrLog( << "MySQL store result failed: error=" << mysql_errno(mConn) << ": " << mysql_error(mConn));
     return rec;
  }

  MYSQL_ROW row=mysql_fetch_row(result);
  if(row)
   {
       int col = 0;
       rec.mIdUser            = Data(row[col++]).convertInt();
       rec.mIdRealm         = Data(row[col++]).convertInt();
       rec.mPassword          = Data(row[col++]);
   }
  mysql_free_result(result);
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

/*************************************************************************/
/*                        REGISTRAR                                      */
/*************************************************************************/
bool
RegDB::addRegistrar(const Key& key, const RegistrarRecord& rec)
{
  Data command;
  {
     DataStream ds(command);
     ds << "INSERT INTO tregistrar (fiduser, fiddomain, fcallid, fidmain)"
        << " VALUES("
        << rec.mIdUser << ", "
        << rec.mIdDomain << ", "
        << rec.mCallId<<", "
        << rec.mIdMain << ")";

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
  RegistrarRecord rec;
  Data command;
  {
     DataStream ds(command);
     ds << "SELECT fiduser, fiddomain, fcallid, fidmain FROM tregistrar"
        << " WHERE fidreg='" << key
        << "'";
  }

  MYSQL_RES* result = 0;
  if(query(command, &result) != 0)
  {
     return rec;
  }
  if (result == 0)
  {
     ErrLog( << "MySQL store result failed: error=" << mysql_errno(mConn) << ": " << mysql_error(mConn));
     return rec;
  }

  MYSQL_ROW row=mysql_fetch_row(result);
  if(row)
   {
       int col = 0;
       rec.mIdReg             = Data(key).convertInt();
       rec.mIdUser            = Data(row[col++]).convertInt();
       rec.mIdDomain          = Data(row[col++]).convertInt();
       rec.mCallId            = Data(row[col++]);
       rec.mIdMain            = Data(row[col++]).convertInt();
   }
  mysql_free_result(result);
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
/*************************************************************************/
/*                        ROUTE                                          */
/*************************************************************************/
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
        << rec.mExpires << ")";
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
  RouteRecord rec;
  Data command;
  {
     DataStream ds(command);
     ds << "SELECT fidreg, fidforward, ftime, fexpires FROM troute"
        << " WHERE fidroute='" << key
        << "'";
  }

  MYSQL_RES* result = 0;
  if(query(command, &result) != 0)
  {
     return rec;
  }
  if (result == 0)
  {
     ErrLog( << "MySQL store result failed: error=" << mysql_errno(mConn) << ": " << mysql_error(mConn));
     return rec;
   }

  MYSQL_ROW row=mysql_fetch_row(result);
  if(row)
   {
       int col = 0;
       rec.mIdRoute            = Data(key).convertInt();
       rec.mIdReg              = Data(row[col++]).convertInt();
       rec.mIdForward          = Data(row[col++]).convertInt();
       rec.mTime               = Data(row[col++]);
       rec.mExpires            = Data(row[col++]).convertInt();
   }
  mysql_free_result(result);
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
     //UPDATE t1 SET c=c+1 WHERE a=1;
     DataStream ds(command);
     ds << "UPDATE troute SET"
        << " fidreg = " << rec.mIdReg
        << ", fidforward = " << rec.mIdForward
        << ", ftime = " << rec.mTime
        << ", fexpires = " << rec.mExpires
        << " WHERE fidroute = " << key;
  }
  return query(command, 0) == 0;
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
