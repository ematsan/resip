#include "RegDB.hxx"
#include "rutil/Data.hxx"
#include "rutil/Logger.hxx"

using namespace registrar;
using namespace resip;

#define RESIPROCATE_SUBSYSTEM Subsystem::TEST

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
     ds << "SELECT fname FROM tuser"
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
/*                       USER DOMAIN                                     */
/*************************************************************************/
bool
RegDB::addUserDomain(const UserDomainRecord& rec)
{
  Data command;
  {
     DataStream ds(command);
     ds << "INSERT INTO tuserdomain (fiduserfk, fiddomainfk)"
        << " VALUES('"
        << rec.mIdUserFk << ","
        << rec.mIdDomainFk<<"')";
  }
  return query(command, 0) == 0;
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
       rec.mIdUD          = Data(key).convertInt();
       rec.mIdDomainFk       = Data(row[col++]).convertInt();
       rec.mIdUserFk          = Data(row[col++]).convertInt();
   }
  mysql_free_result(result);
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
     ds << "SELECT fidprotocolfk, fiddomainfk, fport FROM tforward"
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
       rec.mIdProtocolFk            = Data(row[col++]).convertInt();
       rec.mIdDomainFk               = Data(row[col++]).convertInt();
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
RegDB::addAuthorization(const AuthorizationRecord& rec)
{
  Data command;
  {
     DataStream ds(command);
     ds << "INSERT INTO tauthorization (fidudfk, fpassword)"
        << " VALUES("
        << rec.mIdUDFk << ", '"
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
     ds << "SELECT fidudfk, fpassword FROM tauthorization"
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
       rec.mIdAuth            = Data(key).convertInt();
       rec.mIdUDFk            = Data(row[col++]).convertInt();
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
RegDB::addRegistrar(const RegistrarRecord& rec)
{
  Data command;
  {
     DataStream ds(command);
     ds << "INSERT INTO tregistrar (fidudfk, fcallid, fidmainfk)"
        << " VALUES("
        << rec.mIdUDFk << ", '"
        << rec.mCallId<<"', "
        << rec.mIdMainFk << ")";
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
     ds << "SELECT fidudfk, fcallid, fidmainfk FROM tregistrar"
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
       rec.mIdUDFk            = Data(row[col++]).convertInt();
       rec.mCallId            = Data(row[col++]);
       rec.mIdMainFk          = Data(row[col++]).convertInt();
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
  return query(command, 0) == 0;
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
     //ds << "SELECT fidreg, fidforward, UNIX_TIMESTAMP(ftime), fexpires FROM troute"
     ds << "SELECT fidregfk, fidforwardfk, ftime, fexpires FROM troute"
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
       rec.mIdRegFk              = Data(row[col++]).convertInt();
       rec.mIdForwardFk          = Data(row[col++]).convertInt();
       rec.mTime               = Data(row[col++]);
       //https://stackoverflow.com/questions/38100936/convert-mysql-datetime-to-c-stdtime-t
       //std::time_t t =  res->getInt("UNIX_TIMESTAMP(ftime)");
       //std::time_t t = rec.mTime.convertInt();
       //https://www.tutorialspoint.com/cplusplus/cpp_date_time.htm
       //tm *ltm = localtime(&t);
       // print various components of tm structure.
       //http://mycpp.ru/cpp/scpp/cppd_datetime.htm
       /*cout << "Year:" << 1900 + ltm->tm_year<<endl;
       cout << "Month: "<< 1 + ltm->tm_mon<< endl;
       cout << "Day: "<<  ltm->tm_mday << endl;
       cout << "Time: "<< ltm->tm_hour << ":";
       cout << ltm->tm_min << ":";
       cout << ltm->tm_sec << endl;*/
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
     DataStream ds(command);
     ds << "UPDATE troute SET"
        << " fidregfk = " << rec.mIdRegFk
        << ", fidforwardfk = " << rec.mIdForwardFk
        << ", ftime = '" << rec.mTime
        << "', fexpires = " << rec.mExpires
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
