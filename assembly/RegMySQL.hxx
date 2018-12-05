#if !defined(REGMYSQL_HXX)
#define REGMYSQL_HXX
#include "RegDB.hxx"

#include <mysql/mysql.h>
#include <mysql/errmsg.h>

using namespace std;

#define RESIPROCATE_SUBSYSTEM Subsystem::TEST

namespace resip
{
  class Data;
}

namespace registrar
{
class RegMySQL: public RegDB
{
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


  private:
    virtual resip::Data dbKey(const Table table, bool first = false);
    virtual int query(const resip::Data& queryCommand, MYSQL_RES** result) const;
    virtual int query(const resip::Data& queryCommand) const;

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
