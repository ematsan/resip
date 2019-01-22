#include <iostream>
#include <memory>

#include "rutil/Data.hxx"
#include "rutil/Logger.hxx"

#include "RegMySQL.hxx"

using namespace std;
using namespace resip;
using namespace registrar;

RegMySQL::RegMySQL(const resip::Data& server,
          const resip::Data& user,
          const resip::Data& password,
          const resip::Data& databaseName,
          unsigned int port)
    :mDBServer(server)
    ,mDBUser(user)
    ,mDBPassword(password)
    ,mDBName(databaseName)
    ,mDBPort(port)
    ,mConn(nullptr)
    {
      InfoLog(<<"RegMySQL constructor");
      InfoLog( << "Using MySQL DB with server=" << server << ", user=" << user << ", dbName=" << databaseName << ", port=" << port);

      mysql_library_init(0, 0, 0);

      if(!mysql_thread_safe()){
         ErrLog( << "Registrar uses MySQL from multiple threads - you MUST link with a thread safe version of the mySQL client library!");
      }
      else{
         connectDB();
      }

}

RegMySQL::~RegMySQL(){
    InfoLog(<<"RegMySQL destructor");
    try
    {
      if(mConn)
          disconnectDB();
    }
    catch(std::exception const& a)
    {
        ErrLog(<<"RegMySQL destructor Throw Exeption: " << a.what());
    }
}

int
RegMySQL::connectDB() const {
  // Disconnect from database first (if required)
  disconnectDB();

  // Now try to connect
  resip_assert(mConn == nullptr);
  resip_assert(mConnected == false);

  mConn = mysql_init(0);
  if(mConn == nullptr)
  {
     ErrLog( << "MySQL init failed: insufficient memory.");
     return CR_OUT_OF_MEMORY;
  }

  MYSQL* ret = mysql_real_connect(mConn,
                                  mDBServer.c_str(),   // hostname
                                  mDBUser.c_str(),     // user
                                  mDBPassword.c_str(), // password
                                  mDBName.c_str(),     // DB
                                  mDBPort,             // port
                                  0,                   // unix socket file
                                  CLIENT_MULTI_RESULTS); // client flags (enable multiple results, since some custom stored procedures might require this)

  if (ret == nullptr)
  {
     int rc = mysql_errno(mConn);
     ErrLog( << "MySQL connect failed: error=" << rc << ": " << mysql_error(mConn));
     mysql_close(mConn);
     mConn = nullptr;
     mConnected = false;
     return rc;
  }
  else
  {
     InfoLog( << "MySQL connect ok");
     mConnected = true;
     return 0;
  }

}

void
RegMySQL::shutdown()
{
  if(mConn)
      disconnectDB();
}

void
RegMySQL::disconnectDB() const
{
  if(mConn)
  {
     mysql_close(mConn);
     mConn = nullptr;
     mConnected = false;
  }
}

/*************************************************************************/
/*                        getAll                                          */
/*************************************************************************/

RegDB::UserRecordList
RegMySQL::getAllUsers() const
{
  UserRecordList records;

  MYSQL_RES* result;
  Data command("SELECT fiduser, fname FROM tuser");

  if(query(command, &result) != 0)
  {
     return records;
  }
  if (result == nullptr)
  {
     ErrLog( << "Base store UserRecordList result failed");
     return records;
  }

  MYSQL_ROW row;
  UserRecord rec;
  int col;
  while(row = mysql_fetch_row(result))
   {
       col = 0;
       rec.mIdUser          = Data(row[col++]).convertInt();
       rec.mName            = Data(row[col++]);
       records.push_back(rec);
   }
  mysql_free_result(result);

  return records;
}

RegDB::DomainRecordList
RegMySQL::getAllDomains() const
{
  DomainRecordList records;
  MYSQL_RES* result;
  Data command("SELECT fiddomain, fdomain, fidrealm FROM tdomain");

  if(query(command, &result) != 0)
  {
     return records;
  }
  if (result == nullptr)
  {
     ErrLog( << "Base store DomainRecordList result failed");
     return records;
  }

  MYSQL_ROW row;
  DomainRecord rec;
  int col;
  while(row = mysql_fetch_row(result))
   {
       col = 0;
       rec.mIdDomain          = Data(row[col++]).convertInt();
       rec.mDomain            = Data(row[col++]);
       rec.mIdRealm          = Data(row[col++]).convertInt();
       records.push_back(rec);
   }
  mysql_free_result(result);

  return records;
}

RegDB::UserDomainRecordList
RegMySQL::getAllUserDomains() const
{
  UserDomainRecordList records;
  MYSQL_RES* result;
  Data command("SELECT fidud, fiddomainfk, fiduserfk FROM tuserdomain");

  if(query(command, &result) != 0)
  {
     return records;
  }
  if (result == nullptr)
  {
     ErrLog( << "Base store UserDomainRecordList result failed");
     return records;
  }

  MYSQL_ROW row;
  UserDomainRecord rec;
  int col;
  while(row = mysql_fetch_row(result))
   {
       col = 0;
       rec.mIdUserDomain          = Data(row[col++]).convertInt();
       rec.mIdDomainFk          = Data(row[col++]).convertInt();
       rec.mIdUserFk          = Data(row[col++]).convertInt();
       records.push_back(rec);
   }
  mysql_free_result(result);

  return records;
}

RegDB::ProtocolRecordList
RegMySQL::getAllProtocols() const
{
  ProtocolRecordList records;
  MYSQL_RES* result;
  Data command("SELECT fidprotocol, fprotocol FROM tprotocol");

  if(query(command, &result) != 0)
  {
     return records;
  }
  if (result == nullptr)
  {
     ErrLog( << "Base store ProtocolRecordList result failed");
     return records;
  }

  MYSQL_ROW row;
  ProtocolRecord rec;
  int col;
  while(row = mysql_fetch_row(result))
   {
       col = 0;
       rec.mIdProtocol          = Data(row[col++]).convertInt();
       rec.mProtocol          = Data(row[col++]);
       records.push_back(rec);
   }
  mysql_free_result(result);

  return records;
}

RegDB::ForwardRecordList
RegMySQL::getAllForwards() const
{
  ForwardRecordList records;
  MYSQL_RES* result;
  Data command("SELECT fidforward, fidprotocolfk, fiddomainfk, fport FROM tforward");

  if(query(command, &result) != 0)
  {
     return records;
  }
  if (result == nullptr)
  {
     ErrLog( << "Base store ForwardRecordList result failed");
     return records;
  }

  MYSQL_ROW row;
  ForwardRecord rec;
  int col;
  while(row = mysql_fetch_row(result))
   {
       col = 0;
       rec.mIdForward          = Data(row[col++]).convertInt();
       rec.mIdProtocolFk       = Data(row[col++]).convertInt();
       rec.mIdDomainFk         = Data(row[col++]).convertInt();
       rec.mPort          = Data(row[col++]).convertInt();
       records.push_back(rec);
   }
  mysql_free_result(result);


  return records;
}

RegDB::AuthorizationRecordList
RegMySQL::getAllAuthorizations() const
{
  AuthorizationRecordList records;
  MYSQL_RES* result;
  Data command("SELECT fidauth, fidudfk, fpassword FROM tauthorization");

  if(query(command, &result) != 0)
  {
     return records;
  }
  if (result == nullptr)
  {
     ErrLog( << "Base store AuthorizationRecordList result failed");
     return records;
  }

  MYSQL_ROW row;
  AuthorizationRecord rec;
  int col;
  while(row = mysql_fetch_row(result))
   {
       col = 0;
       rec.mIdAuth          = Data(row[col++]).convertInt();
       rec.mIdUserDomainFk  = Data(row[col++]).convertInt();
       rec.mPassword        = Data(row[col++]);
       records.push_back(rec);
   }
  mysql_free_result(result);

  return records;
}

RegDB::RegistrarRecordList
RegMySQL::getAllRegistrars() const
{
  RegistrarRecordList records;
  MYSQL_RES* result;
  Data command("SELECT fidreg, fidudfk, fcallid, fidmainfk FROM tregistrar");

  if(query(command, &result) != 0)
  {
     return records;
  }
  if (result == nullptr)
  {
     ErrLog( << "Base store RegistrarRecordList result failed");
     return records;
  }

  MYSQL_ROW row;
  RegistrarRecord rec;
  int col;
  while(row = mysql_fetch_row(result))
   {
       col = 0;
       rec.mIdReg          = Data(row[col++]).convertInt();
       rec.mIdUserDomainFk  = Data(row[col++]).convertInt();
       rec.mCallId        = Data(row[col++]);
       rec.mIdMainFk  = Data(row[col++]).convertInt();
       records.push_back(rec);
   }
  mysql_free_result(result);

  return records;
}

RegDB::RouteRecordList
RegMySQL::getAllRoutes() const
{
  RouteRecordList records;
  MYSQL_RES* result;
  Data command("SELECT fidroute, fidregfk, fidforwardfk, ftime, fexpires FROM troute");

  if(query(command, &result) != 0)
  {
     return records;
  }
  if (result == nullptr)
  {
     ErrLog( << "Base store RouteRecordList result failed");
     return records;
  }

  MYSQL_ROW row;
  RouteRecord rec;
  int col;
  while(row = mysql_fetch_row(result))
   {
       col = 0;
       rec.mIdRoute          = Data(row[col++]).convertInt();
       rec.mIdRegFk  = Data(row[col++]).convertInt();
       rec.mIdForwardFk  = Data(row[col++]).convertInt();
       rec.mTime        = Data(row[col++]);
       rec.mExpires  = Data(row[col++]).convertInt();
       records.push_back(rec);
   }
  mysql_free_result(result);

  return records;
}
/*************************************************************************/
/*                        query                                          */
/*************************************************************************/
int
RegMySQL::query(const resip::Data& queryCommand, UserRecord& rec) const
{
MYSQL_RES* result;
if(query(queryCommand, &result) != 0)
{
   return -2;
}
if (result == nullptr)
{
   ErrLog( << "Base store UserRecord result failed");
   return -1;
}

MYSQL_ROW row=mysql_fetch_row(result);
if(row)
 {
     int col = 0;
     rec.mIdUser          = Data(row[col++]).convertInt();
     rec.mName            = Data(row[col++]);
 }
mysql_free_result(result);
return 0;
}

int
RegMySQL::query(const resip::Data& queryCommand, DomainRecord& rec) const
{
  MYSQL_RES* result;
  if(query(queryCommand, &result) != 0)
  {
     return -2;
  }
  if (result == nullptr)
  {
     ErrLog( << "Base store DomainRecord result failed");
     return -1;
  }

  MYSQL_ROW row=mysql_fetch_row(result);
  if(row)
   {
       int col = 0;
       rec.mIdDomain          = Data(row[col++]).convertInt();
       rec.mDomain          = Data(row[col++]);
       rec.mIdRealm          = Data(row[col++]).convertInt();
   }
  mysql_free_result(result);
  return 0;
}

int
RegMySQL::query(const resip::Data& queryCommand, UserDomainRecord& rec) const
{
  MYSQL_RES* result;
    if(query(queryCommand, &result) != 0)
    {
       return -2;
    }
    if (result == nullptr)
    {
       ErrLog( << "Base store UserDomainRecord result failed");
       return -1;
    }

    MYSQL_ROW row=mysql_fetch_row(result);
    if(row)
     {
         int col = 0;
         rec.mIdUserDomain          = Data(row[col++]).convertInt();
         rec.mIdDomainFk       = Data(row[col++]).convertInt();
         rec.mIdUserFk          = Data(row[col++]).convertInt();
     }
    mysql_free_result(result);
    return 0;
}

int
RegMySQL::query(const resip::Data& queryCommand, ProtocolRecord& rec) const
{
  MYSQL_RES* result;
    if(query(queryCommand, &result) != 0)
    {
       return -2;
    }
    if (result == nullptr)
    {
       ErrLog( << "Base store ProtocolRecord result failed");
       return -1;
    }

    MYSQL_ROW row=mysql_fetch_row(result);
    if(row)
     {
         int col = 0;
         rec.mIdProtocol          = Data(row[col++]).convertInt();
         rec.mProtocol            = Data(row[col++]);
     }
    mysql_free_result(result);
    return 0;
}

int
RegMySQL::query(const resip::Data& queryCommand, AuthorizationRecord& rec) const
{
  MYSQL_RES* result;
  if(query(queryCommand, &result) != 0)
  {
     return -2;
  }
  if (result == nullptr)
  {
     ErrLog( << "Base store AuthorizationRecord result failed");
     return -1;
  }

  MYSQL_ROW row=mysql_fetch_row(result);
  if(row)
   {
       int col = 0;
       rec.mIdAuth            = Data(row[col++]).convertInt();
       rec.mIdUserDomainFk    = Data(row[col++]).convertInt();
       rec.mPassword          = Data(row[col++]);
   }
  mysql_free_result(result);
  return 0;
}

int
RegMySQL::query(const resip::Data& queryCommand, ForwardRecord& rec) const
{
  MYSQL_RES* result;
  if(query(queryCommand, &result) != 0)
  {
     return -2;
  }
  if (result == nullptr)
  {
     ErrLog( << "Base store ForwardRecord result failed");
     return -1;
  }

  MYSQL_ROW row=mysql_fetch_row(result);
  if(row)
   {
       int col = 0;
       rec.mIdForward            = Data(row[col++]).convertInt();
       rec.mIdProtocolFk            = Data(row[col++]).convertInt();
       rec.mIdDomainFk               = Data(row[col++]).convertInt();
       rec.mPort                  = Data(row[col++]).convertInt();
   }
  mysql_free_result(result);
  return 0;
}

int
RegMySQL::query(const resip::Data& queryCommand, RegistrarRecord& rec) const
{
  MYSQL_RES* result;
    if(query(queryCommand, &result) != 0)
    {
       return -2;
    }
    if (result == nullptr)
    {
       ErrLog( << "Base store RegistrarRecord result failed");
       return -1;
    }

    MYSQL_ROW row=mysql_fetch_row(result);
    if(row)
     {
         int col = 0;
         rec.mIdReg             = Data(row[col++]).convertInt();
         rec.mIdUserDomainFk    = Data(row[col++]).convertInt();
         rec.mCallId            = Data(row[col++]);
         rec.mIdMainFk          = Data(row[col++]).convertInt();
     }
    mysql_free_result(result);
    return 0;
}

int
RegMySQL::query(const resip::Data& queryCommand, RouteRecord& rec) const
{
  MYSQL_RES* result;
    if(query(queryCommand, &result) != 0)
    {
       return -2;
    }
    if (result == nullptr)
    {
       ErrLog( << "Base store RouteRecord result failed");
       return -1;
     }

    MYSQL_ROW row=mysql_fetch_row(result);
    if(row)
     {
         int col = 0;
         rec.mIdRoute            = Data(row[col++]).convertInt();
         rec.mIdRegFk              = Data(row[col++]).convertInt();
         rec.mIdForwardFk          = Data(row[col++]).convertInt();
         rec.mTime               = Data(row[col++]);
         rec.mExpires            = Data(row[col++]).convertInt();
     }
    mysql_free_result(result);
    return 0;
}

int
RegMySQL::query(const Data& queryCommand, MYSQL_RES** result) const
{
   int rc = 0;
   DebugLog( << "RegMySQL::query: executing query: " << queryCommand);
   std::lock_guard<std::mutex> lk(mMutex);
   if(mConn == nullptr || !mConnected)    rc = connectDB();
   if(rc == 0)
   {
      resip_assert(mConn!=nullptr);
      resip_assert(mConnected);
      rc = mysql_query(mConn,queryCommand.c_str());
      if(rc != 0)
      {
         rc = mysql_errno(mConn);
         if(rc == CR_SERVER_GONE_ERROR ||
            rc == CR_SERVER_LOST)
         {
            // First failure is a connection error - try to re-connect and then try again
            rc = connectDB();
            if(rc == 0)
            {
               // OK - we reconnected - try query again
               rc = mysql_query(mConn,queryCommand.c_str());
               if( rc != 0)
               {
                  ErrLog( << "MySQL query failed: error=" << mysql_errno(mConn) << ": " << mysql_error(mConn));
               }
            }
         }
         else  {
            ErrLog( << "MySQL query failed: error=" << mysql_errno(mConn) << ": " << mysql_error(mConn));
         }
      }
   }
   // Now store result - if pointer to result pointer was supplied and no errors
   if(rc == 0 && result)  {
      *result = mysql_store_result(mConn);
      if(*result == 0)
      {
         rc = mysql_errno(mConn);
         if(rc != 0)
         {
            ErrLog( << "MySQL store result failed: error=" << rc << ": " << mysql_error(mConn));
         }
      }
   }
   if(rc != 0) {
      ErrLog( << " SQL Command was: " << queryCommand) ;
   }
   return rc;
}

int
RegMySQL::query(const Data& queryCommand) const
{
  return query(queryCommand, 0);
}
