#if !defined(REGMYSQL_HXX)
#define REGMYSQL_HXX

#include <mysql/mysql.h>
#include <mysql/errmsg.h>

#define RESIPROCATE_SUBSYSTEM Subsystem::TEST

namespace resip
{
  class Data;
}

namespace registrar
{
class RegMySQL{
  public:

    RegMySQL(const resip::Data& server,
            const resip::Data& user,
            const resip::Data& password,
            const resip::Data& databaseName,
            unsigned int port);
    ~RegMySQL();

    int connectDB() const;
    void disconnectDB() const;
    void initialize() const;
    int query(const resip::Data& queryCommand, MYSQL_RES** result) const;
    int query(const resip::Data& queryCommand) const;
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

}

#endif
