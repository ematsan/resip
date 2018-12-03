#include <iostream>
#include <memory>

#include "rutil/Data.hxx"
#include "rutil/Logger.hxx"

#include "RegMySQL.hxx"
//#include "RegDB.hxx"

using namespace std;
using namespace resip;
using namespace registrar;

void mysqlThreadEnd(void*){ mysql_thread_end();}
class MySQLInitializer
{
   public:
      MySQLInitializer()
      {
         cout<<"MySQLInitializer constructor"<<endl;
         ThreadIf::tlsKeyCreate(mThreadStorage, mysqlThreadEnd);
      }
      ~MySQLInitializer()
      {
         cout<<"MySQLInitializer destructor"<<endl;
         ThreadIf::tlsKeyDelete(mThreadStorage);
      }
      void setInitialized()
      {
         ThreadIf::tlsSetValue(mThreadStorage, (void*) true);
      }
      bool isInitialized()
      {
         // Note:  if value is not set yet then 0 (false) is returned
         return ThreadIf::tlsGetValue(mThreadStorage) != 0;
      }

   private:
      ThreadIf::TlsKey mThreadStorage;
};
static MySQLInitializer g_MySQLInitializer;




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
      cout<<"RegMySQL constructor"<<endl;
      InfoLog( << "Using MySQL DB with server=" << server << ", user=" << user << ", dbName=" << databaseName << ", port=" << port);

      for (int i=0;i<MaxTable;i++)
      {
         mResult[i]=0;
      }

      mysql_library_init(0, 0, 0);
      if(!mysql_thread_safe()){
         ErrLog( << "Repro uses MySQL from multiple threads - you MUST link with a thread safe version of the mySQL client library!");
      }
      else{
         connectDB();
      }
}

RegMySQL::~RegMySQL(){
    cout<<"RegMySQL destructor"<<endl;
    disconnectDB();
    //mysql_library_end();
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


void
RegMySQL::initialize() const{
   if(!g_MySQLInitializer.isInitialized())
   {
      g_MySQLInitializer.setInitialized();
      mysql_thread_init();
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
RegMySQL::query(const Data& queryCommand, MYSQL_RES** result) const
{
   int rc = 0;
   initialize();
   DebugLog( << "RegMySQL::query: executing query: " << queryCommand);
   Lock lock(mMutex);
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
