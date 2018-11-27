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
}

~MySQLDB(){
  disconnectDB();
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
     DebugLog( << "MySQL connect ok");
     mConnected = true;
     return 0;
  }
}

void disconnectDB() const{
  if(mConn)
  {
    /* for (int i=0; i<MaxTable; i++)
     {
        if (mResult[i])
        {
           mysql_free_result(mResult[i]);
           mResult[i]=0;
        }
     }*/

     mysql_close(mConn);
     mConn = 0;
     mConnected = false;
  }
}

private:
  resip::Data mDBServer;
  resip::Data mDBUser;
  resip::Data mDBPassword;
  resip::Data mDBName;
  unsigned int mDBPort;

  mutable bool mConnected;

  mutable MYSQL* mConn;

};

int
main(int argc, char* argv[])
{
  cout << "test"<<endl;
  MySQLDB* base = new MySQLDB("localhost","repro","Net0Par0!ya","repro",3306);
  if (!base->connectDB())
     cout << "ok"<<endl;
  return 0;
}
