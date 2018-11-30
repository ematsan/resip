#include "iostream"
#include <mysql/mysql.h>
#include <mysql/errmsg.h>

//#include <sys/types.h>
#include <iostream>
#include <memory>

#include "rutil/Data.hxx"
#include "rutil/Logger.hxx"

using namespace std;
using namespace resip;

#define RESIPROCATE_SUBSYSTEM Subsystem::NONE

// This class helps ensure that each thread using the MySQL API's
// initialize by calling mysql_thread_init before calling any mySQL functions

void mysqlThreadEnd(void*){ mysql_thread_end();}
class MySQLInitializer
{
   public:
      MySQLInitializer()
      {
         ThreadIf::tlsKeyCreate(mThreadStorage, mysqlThreadEnd);
      }
      ~MySQLInitializer()
      {
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


class MySQLDB{
public:
  MySQLDB(const resip::Data& server,
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

~MySQLDB(){
    disconnectDB();
    //mysql_library_end();
}

int connectDB() const{
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

void disconnectDB() const{
  if(mConn)
  {
     for (int i=0; i<MaxTable; i++)
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


void initialize() const{
   if(!g_MySQLInitializer.isInitialized())
   {
      g_MySQLInitializer.setInitialized();
      mysql_thread_init();
   }
}

int query(const Data& queryCommand, MYSQL_RES** result) const
{
   int rc = 0;
   initialize();
   DebugLog( << "MySqlDb::query: executing query: " << queryCommand);
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

int query(const Data& queryCommand) const{ return query(queryCommand, 0); }

private:

  typedef enum
  {
     UserTable=0,
     RouteTable,
     AclTable,
     ConfigTable,
     StaticRegTable,
     FilterTable,
     SiloTable,
     MaxTable  // This one MUST be last
  } Table;

  resip::Data mDBServer;
  resip::Data mDBUser;
  resip::Data mDBPassword;
  resip::Data mDBName;
  unsigned int mDBPort;

  mutable bool mConnected;

  mutable MYSQL* mConn;
  mutable MYSQL_RES* mResult[MaxTable];

  // when multiple threads are in use with the same connection, you need to
  // mutex calls to mysql_query and mysql_store_result:
  // http://dev.mysql.com/doc/refman/5.1/en/threaded-clients.html
  mutable resip::Mutex mMutex;

};

int
main(int argc, char* argv[])
{
  //read data from config file
  MySQLDB* base = new MySQLDB("localhost","repro","Net0Par0!ya","repro",3306);
  MYSQL_RES* result=0;
  
  Data str = "SELECT * FROM users";

  if(base->query(str, &result) != 0)
     ErrLog( << "MySQL store have no resalt");
  if (result==0)
     ErrLog( << "MySQL store result failed");

  MYSQL_ROW row;
//http://www.mysql.ru/docs/man/mysql_fetch_row.html
  unsigned int num_fields = mysql_num_fields(result);
  while ((row = mysql_fetch_row(result)))
  {
     for(int i = 0; i < num_fields; i++){
        cout<<"["<<row[i]<<"]  ";
      }
     cout<<endl;
  }
  mysql_free_result(result);
  return 0;
}
