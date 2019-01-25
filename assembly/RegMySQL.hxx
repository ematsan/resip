#pragma once
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

    class MySQLError{};

    RegMySQL(const resip::Data& server,
            const resip::Data& user,
            const resip::Data& password,
            const resip::Data& databaseName,
            unsigned int port);
    virtual ~RegMySQL();

    virtual int connectDB() const override;
    virtual void disconnectDB() const override;
    virtual void shutdown() override;

    // functions for User Records
    virtual bool addUser(const UserRecord& rec) const override;
    virtual bool eraseUser(const Key& key) const override;
    virtual int findUserId(const UserRecord& rec) const override;
    virtual UserRecord getUser(const Key& key) const override;
    virtual UserRecordList getAllUsers() const override;
    // functions for Domain Records
    virtual bool addDomain(const DomainRecord& rec) const override;
    virtual bool eraseDomain(const Key& key) const override;
    virtual DomainRecord getDomain(const Key& key) const override;
    virtual int findDomainId(const DomainRecord& rec) const override;
    virtual DomainRecordList getAllDomains() const override;
    // functions for User Domain Records
    virtual bool addUserDomain(const UserDomainRecord& rec) const override;
    virtual bool eraseUserDomain(const Key& key) const override;
    virtual UserDomainRecord getUserDomain(const Key& key) const override;
    virtual int findUserDomainId(const UserDomainRecord& rec) const override;
    virtual UserDomainRecordList getAllUserDomains()  const  override;
    // functions for Protocol Records
    virtual bool addProtocol(const ProtocolRecord& rec) const override;
    virtual bool eraseProtocol(const Key& key) const override;
    virtual ProtocolRecord getProtocol(const Key& key) const override;
    virtual int findProtocolId(const ProtocolRecord& rec) const override;
    virtual ProtocolRecordList getAllProtocols()  const override;
    // functions for Authorization Records
    virtual bool addAuthorization(const AuthorizationRecord& rec) const override;
    virtual bool eraseAuthorization(const Key& key) const override;
    virtual AuthorizationRecord getAuthorization(const Key& key) const override;
    virtual int findAuthorizationId(const AuthorizationRecord& rec) const override;
    virtual AuthorizationRecordList getAllAuthorizations() const override;
    // functions for Forward Records
    virtual bool addForward(const ForwardRecord& rec) const override;
    virtual bool eraseForward(const Key& key) const override;
    virtual ForwardRecord getForward(const Key& key) const override;
    virtual int findForwardId(const ForwardRecord& rec) const override;
    virtual ForwardRecordList getAllForwards() const override;
    // functions for Registrar Records
    virtual bool addRegistrar(const RegistrarRecord& rec) const override;
    virtual bool eraseRegistrar(const Key& key) const override;
    virtual RegistrarRecord getRegistrar(const Key& key) const override;
    virtual bool updateRegistrar(const Key& key, const RegistrarRecord& rec) const override;
    virtual int findRegistrarId(const RegistrarRecord& rec) const override;
    virtual RegistrarRecordList getAllRegistrars() const override;
    // functions for Route Records
    virtual bool addRoute(const RouteRecord& rec) const override;
    virtual bool eraseRoute(const Key& key) const override;
    virtual RouteRecord getRoute(const Key& key) const override;
    virtual bool updateRoute(const Key& key, const RouteRecord& rec) const override;
    virtual int findRouteId(const RouteRecord& rec) const override;
    virtual RouteRecordList getAllRoutes()  const override;


protected:

private:
    virtual int query(const resip::Data& queryCommand, MYSQL_RES** result) const;
    virtual int query(const resip::Data& queryCommand) const;


    resip::Data mDBServer;
    resip::Data mDBUser;
    resip::Data mDBPassword;
    resip::Data mDBName;
    unsigned int mDBPort;

    mutable MYSQL* mConn;
    mutable std::mutex mMutex;
};
}
