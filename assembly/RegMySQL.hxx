#if !defined(REGMYSQL_HXX)
#define REGMYSQL_HXX
#include "RegDB.hxx"

#include <mysql/mysql.h>
#include <mysql/errmsg.h>
#include <mutex>

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

    virtual int connectDB() const override;
    virtual void disconnectDB() const override;
    virtual void shutdown() override;

  private:
    virtual int query(const resip::Data& queryCommand, MYSQL_RES** result) const;

    virtual int query(const resip::Data& queryCommand, UserRecord& result) const override;
    virtual UserRecordList getAllUsers() const override;
    virtual int query(const resip::Data& queryCommand, DomainRecord& result) const override;
    virtual DomainRecordList getAllDomains() const override;
    virtual int query(const resip::Data& queryCommand, UserDomainRecord& result) const override;
    virtual UserDomainRecordList getAllUserDomains() const override;
    virtual int query(const resip::Data& queryCommand, ProtocolRecord& result) const override;
    virtual ProtocolRecordList getAllProtocols() const override;
    virtual int query(const resip::Data& queryCommand, AuthorizationRecord& result) const override;
    virtual AuthorizationRecordList getAllAuthorizations() const override;
    virtual int query(const resip::Data& queryCommand, ForwardRecord& result) const override;
    virtual ForwardRecordList getAllForwards() const override;
    virtual int query(const resip::Data& queryCommand, RegistrarRecord& result) const override;
    virtual RegistrarRecordList getAllRegistrars() const override;
    virtual int query(const resip::Data& queryCommand, RouteRecord& result) const override;
    virtual RouteRecordList getAllRoutes() const override;

    virtual int query(const resip::Data& queryCommand) const override;

    resip::Data mDBServer;
    resip::Data mDBUser;
    resip::Data mDBPassword;
    resip::Data mDBName;
    unsigned int mDBPort;

    mutable bool mConnected;
    mutable MYSQL* mConn;

  // when multiple threads are in use with the same connection, you need to
  // mutex calls to mysql_query and mysql_store_result:
    mutable std::mutex mMutex;

};
}
#endif
