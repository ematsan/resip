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
    ,mConn(0)
    ,mConnected(false){
      InfoLog(<<"RegMySQL constructor");
      InfoLog( << "Using MySQL DB with server=" << server << ", user=" << user << ", dbName=" << databaseName << ", port=" << port);

      for (int i=0;i<MaxTable;i++)
      {
         mResult[i]=0;
      }
      mysql_library_init(0, 0, 0);
      connectDB();
}

RegMySQL::~RegMySQL(){
    InfoLog(<<"RegMySQL destructor");
    try
    {
        disconnectDB();
    }
    catch(std::exception const& a)
    {
        ErrLog(<<"RegMySQL destructor Throw Exeption: " << a.what());
    }
}

int
RegMySQL::connectDB() const{
  // Disconnect from database first (if required)
  disconnectDB();

  // Now try to connect
  resip_assert(mConn == 0);
  resip_assert(mConnected == false);

  mConn = mysql_init(0);
  if(mConn == 0)
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

  if (ret == 0)
  {
     int rc = mysql_errno(mConn);
     ErrLog( << "MySQL connect failed: error=" << rc << ": " << mysql_error(mConn));
     mysql_close(mConn);
     mConn = 0;
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
  disconnectDB();
}

void
RegMySQL::disconnectDB() const{
  if(mConn)
  {
     for (int i=0; i < MaxTable; i++)
     {
        if (mResult[i])
        {
           mysql_free_result(mResult[i]);
           mResult[i]=0;
        }
     }
     mysql_close(mConn);
     mConn = 0;
     mConnected = false;
  }
}

/*************************************************************************/
/*                        db work                                        */
/*************************************************************************/
// Db manipulation
// find keys
// return empty if no more
resip::Data
RegMySQL::dbKey(const Table table,
                bool first)
{
  //if first select
  if(first)
  {
     // free memory from previous search
     if (mResult[table])
     {
        mysql_free_result(mResult[table]);
        mResult[table] = 0;
     }

     Data command;
     {
        DataStream ds(command);
        ds << "SELECT "<<keyName[table] <<" FROM " << tableName[table];
     }

     if(query(command, &mResult[table]) != 0)
     {
        return Data::Empty;
     }

     if (mResult[table] == 0)
     {
        ErrLog( << "MySQL store result failed: error=" << mysql_errno(mConn) << ": " << mysql_error(mConn));
        return Data::Empty;
     }
  }
  else
  {
     //if we have no more result
     if (mResult[table] == 0)
     {
        return Data::Empty;
     }
  }
  //select next resalt
  MYSQL_ROW row = mysql_fetch_row(mResult[table]);
  //if empty
  if (!row)
  {
     mysql_free_result(mResult[table]);
     mResult[table] = 0;
     return Data::Empty;
  }

  return Data(row[0]);
}

/*************************************************************************/
/*                        query                                          */
/*************************************************************************/
int
RegMySQL::query(const resip::Data& queryCommand, UserRecord& rec, const Key& key) const
{
MYSQL_RES* result = 0;
if(query(queryCommand, &result) != 0)
{
   return -2;
}
if (result == 0)
{
   ErrLog( << "Base store result failed");
   return -1;
}

MYSQL_ROW row=mysql_fetch_row(result);
if(row)
 {
     int col = 0;
     rec.mIdUser          = Data(key).convertInt();
     rec.mName            = Data(row[col++]);
 }
mysql_free_result(result);
return 0;
}

int
RegMySQL::query(const resip::Data& queryCommand, DomainRecord& rec, const Key& key) const
{
  MYSQL_RES* result = 0;
  if(query(queryCommand, &result) != 0)
  {
     return -2;
  }
  if (result == 0)
  {
     ErrLog( << "Base store result failed");
     return -1;
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
  return 0;
}

int
RegMySQL::query(const resip::Data& queryCommand, UserDomainRecord& rec, const Key& key) const
{
  MYSQL_RES* result = 0;
    if(query(queryCommand, &result) != 0)
    {
       return -2;
    }
    if (result == 0)
    {
       ErrLog( << "Base store result failed");
       return -1;
    }

    MYSQL_ROW row=mysql_fetch_row(result);
    if(row)
     {
         int col = 0;
         rec.mIdUserDomain          = Data(key).convertInt();
         rec.mIdDomainFk       = Data(row[col++]).convertInt();
         rec.mIdUserFk          = Data(row[col++]).convertInt();
     }
    mysql_free_result(result);
    return 0;
}

int
RegMySQL::query(const resip::Data& queryCommand, ProtocolRecord& rec, const Key& key) const
{
  MYSQL_RES* result = 0;
    if(query(queryCommand, &result) != 0)
    {
       return -2;
    }
    if (result == 0)
    {
       ErrLog( << "Base store result failed");
       return -1;
    }

    MYSQL_ROW row=mysql_fetch_row(result);
    if(row)
     {
         int col = 0;
         rec.mIdProtocol          = Data(key).convertInt();
         rec.mProtocol            = Data(row[col++]);
     }
    mysql_free_result(result);
    return 0;
}

int
RegMySQL::query(const resip::Data& queryCommand, AuthorizationRecord& rec, const Key& key) const
{
  MYSQL_RES* result = 0;
  if(query(queryCommand, &result) != 0)
  {
     return -2;
  }
  if (result == 0)
  {
     ErrLog( << "Base store result failed");
     return -1;
  }

  MYSQL_ROW row=mysql_fetch_row(result);
  if(row)
   {
       int col = 0;
       rec.mIdAuth            = Data(key).convertInt();
       rec.mIdUserDomainFk    = Data(row[col++]).convertInt();
       rec.mPassword          = Data(row[col++]);
   }
  mysql_free_result(result);
  return 0;
}

int
RegMySQL::query(const resip::Data& queryCommand, ForwardRecord& rec, const Key& key) const
{
  MYSQL_RES* result = 0;
  if(query(queryCommand, &result) != 0)
  {
     return -2;
  }
  if (result == 0)
  {
     ErrLog( << "Base store result failed");
     return -1;
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
  return 0;
}

int
RegMySQL::query(const resip::Data& queryCommand, RegistrarRecord& rec, const Key& key) const
{
  MYSQL_RES* result = 0;
    if(query(queryCommand, &result) != 0)
    {
       return -2;
    }
    if (result == 0)
    {
       ErrLog( << "Base store result failed");
       return -1;
    }

    MYSQL_ROW row=mysql_fetch_row(result);
    if(row)
     {
         int col = 0;
         rec.mIdReg             = Data(key).convertInt();
         rec.mIdUserDomainFk    = Data(row[col++]).convertInt();
         rec.mCallId            = Data(row[col++]);
         rec.mIdMainFk          = Data(row[col++]).convertInt();
     }
    mysql_free_result(result);
    return 0;
}

int
RegMySQL::query(const resip::Data& queryCommand, RouteRecord& rec, const Key& key) const
{
  MYSQL_RES* result = 0;
    if(query(queryCommand, &result) != 0)
    {
       return -2;
    }
    if (result == 0)
    {
       ErrLog( << "Base store result failed");
       return -1;
     }

    MYSQL_ROW row=mysql_fetch_row(result);
    if(row)
     {
         int col = 0;
         rec.mIdRoute            = Data(key).convertInt();
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
   if(mConn == 0 || !mConnected)    rc = connectDB();
   if(rc == 0)
   {
      resip_assert(mConn!=0);
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
