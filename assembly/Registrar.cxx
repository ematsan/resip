#include <iostream>
#include <ctime>
#include <future>
#include "Registrar.hxx"
#include "RegMySQL.hxx"
#include "RegDB.hxx"

#include "resip/stack/Helper.hxx"
#include "resip/stack/SipMessage.hxx"
#include "resip/stack/Uri.hxx"
#include "resip/stack/Auth.hxx"

#include "resip/stack/SipStack.hxx"
#include "rutil/Logger.hxx"

using namespace resip;
using namespace std;
using namespace registrar;


Registrar::Registrar(SipStack& stack, Data realm, RegDB* mdatabase, const vector<Data>& configDomains)
   : mStack(stack)
   , mNameAddr(realm)
   , mBase(mdatabase)
   , mConfigDomains(configDomains)
   , mThreads()
{
  InfoLog(<<"Registrar constructor");
  //select all data
  loadData();
  reloadDomain();
}

Registrar::~Registrar()
{
   InfoLog(<<"Registrar destructor");
   clearData();
}

void
Registrar::reloadDomain()
{
  for (Data domain: mConfigDomains)
  {
    bool kept = false;
    for (RegDB::DomainRecord rec : domainList)
    {
       if (rec.mDomain == domain)
       {
         kept = true;
         break;
       }
    }
    if (!kept)
    {
        RegDB::DomainRecord n;
        n.mDomain = domain;
        mBase->addDomain(n);
        n.mIdDomain = mBase->findDomainId(n);
        if (n.mIdDomain >= 0)
           domainList.push_back(n);
    }
  }

  //left only registrar names
  RegDB::DomainRecordList tlist = domainList;
  domainList.clear();
  for (Data domain: mConfigDomains)
  {
    for (RegDB::DomainRecord rec : tlist)
    {
       if (rec.mDomain == domain)
       {
         domainList.push_back(rec);
         break;
       }
    }
  }
}

void
Registrar::clearData()
{
  userList.clear();
  domainList.clear();
  userDomeinList.clear();
  forwardList.clear();
  protocolList.clear();
  authList.clear();
  regList.clear();
  routeList.clear();
}

void
Registrar::loadData()
{
  clearData();
  auto users = async(&RegDB::getAllUsers, mBase);
  auto domains = async(&RegDB::getAllDomains, mBase);
  auto userDomains = async(&RegDB::getAllUserDomains, mBase);
  auto forwards  = async(&RegDB::getAllForwards, mBase);
  auto protocols  = async(&RegDB::getAllProtocols, mBase);
  auto auths  = async(&RegDB::getAllAuthorizations, mBase);
  auto regs  = async(&RegDB::getAllRegistrars, mBase);
  auto routes  = async(&RegDB::getAllRoutes, mBase);

  userList = users.get();
  if (userList.empty())
  {
    ErrLog(<< "No element in table toUserName");
  }

  domainList = domains.get();
  if (domainList.empty())
  {
    ErrLog(<< "No element in table tDomain");
  }
  userDomeinList = userDomains.get();
  if (userDomeinList.empty())
  {
    ErrLog(<< "No element in table toUserNameDomain");
  }
  forwardList = forwards.get();
  if (forwardList.empty())
  {
    ErrLog(<< "No element in table tForward");
  }
  protocolList = protocols.get();
  if (protocolList.empty())
  {
    ErrLog(<< "No element in table tProtocol");
  }
  authList = auths.get();
  if (authList.empty())
  {
    ErrLog(<< "No element in table tAuthorization");
  }
  regList = regs.get();
  if (regList.empty())
  {
    ErrLog(<< "No element in table tRegistrar");
  }
  routeList = routes.get();
  if (routeList.empty())
  {
    ErrLog(<< "No element in table tRoute");
  }
}

void
Registrar::thread()
{
  InfoLog(<<"This is the Server");
  while (!isShutdown())
  {
     SipMessage* received = mStack.receive();
     if (received)
     {
        MethodTypes meth = received->header(h_RequestLine).getMethod();
        InfoLog(<< "Server received: " << getMethodName(meth));
        if ( meth == REGISTER )
        {
          if (received->exists(h_Authorizations))
           {
             try{
                mThreads.submit(std::bind(&Registrar::analisysRequest, this, *received));
             }
            catch(...)
             {
                send500(received);
             }
          }
          else
          {
              send401(received);
          }
       }
       else
       {
         send405(received, getMethodName(meth));
       }
     }
     delete received;
  }
}

void
//Registrar::analisysRequest(resip::SipMessage* sip)
Registrar::analisysRequest(resip::SipMessage sip)
{
  Auths& auth =  sip.header(h_Authorizations);
  //test Authorization
  if (!testAuthorization(&sip))
  {
     send403(&sip, "User not register");
     return;
   }
  //test Registrar
  unsigned int idreg = findOrAddRegistrar(&sip);
  if (0 == idreg)
  {
     send403(&sip, "User not have access to add record");
     return;
   }

  if(sip.exists(h_Contacts))
        {
           NameAddr& to = sip.header(h_To);
           NameAddr& from = sip.header(h_From);
           CallId& callid = sip.header(h_CallId);

           Data fromUserName = from.uri().user();
           Data fromUserHost = from.uri().host();
           Data toUserName = to.uri().user();
           Data toUserHost = to.uri().host();
           //registration time
           unsigned int expires = 0;
           if (sip.exists(h_Expires))
           {
             expires = sip.header(h_Expires).value();
           }
           else
           {
             expires = 3600;
           }
           //parse all contacts
           ParserContainer<NameAddr>& contacts = sip.header(h_Contacts);

           for (ParserContainer<NameAddr>::iterator i = contacts.begin(); i != contacts.end(); i++)
                   {
                     //test format
                     if (!i->isWellFormed())
                     {
                       send400(&sip);
                       ErrLog(<< "Not well formed");
                       return;
                     }
                     // Check for "Contact: *" style deregistration
                      if (i->isAllContacts())
                      {
                        if (contacts.size() > 1 || expires != 0)
                          {
                              send400(&sip);
                              ErrLog(<< "Error us Contact:*");
                              return;
                           }
                        //remove users contacts
                        removeAllContacts(&sip);
                        return;
                      }

                      NameAddr addr = *i;

                      if (addr.exists(p_expires))
                         expires = addr.param(p_expires);

                      unsigned int idForward = findOrAddForward(addr, expires);
                      if (0 == idForward)
                      {
                         send500(&sip);
                         ErrLog(<< "Not find Forward");
                         return;
                       }
                      //need to delete record
                      bool upd = false;
                      if (0 == expires)
                      {
                        int i = 0;
                        for(RegDB::RouteRecord rec : routeList)
                        {
                          if ((rec.mIdRegFk == idreg)  && (rec.mIdForwardFk == idForward))
                          {
                             mBase->eraseRoute(rec.mIdRoute);
                             upd = true;
                             routeList.erase(routeList.begin() + i);
                           }
                           i++;
                        }
                      }
                      else //need add or update
                      {
                        for(RegDB::RouteRecord rec : routeList)
                        {
                          //if find - update
                          if ((rec.mIdRegFk == idreg)  && (rec.mIdForwardFk == idForward))
                          {
                             rec.mExpires = expires;
                             time_t now = time(0);
                             tm *ltm = localtime(&now);
                             rec.mTime = Data(1900 + ltm->tm_year) + "-" +
                                         Data(1 + ltm->tm_mon) + "-" +
                                         Data(ltm->tm_mday) + " "+
                                         Data(ltm->tm_hour) + ":"+
                                         Data(ltm->tm_min) + ":" +
                                         Data(ltm->tm_sec);
                             mBase->updateRoute(rec.mIdRoute, rec);
                             upd = true;
                           }
                        }
                        //not find - add
                        if (!upd)
                        {
                            RegDB::RouteRecord rec;
                            rec.mIdRegFk = idreg;
                            rec.mIdForwardFk = idForward;
                            rec.mExpires = expires;
                            time_t now = time(0);
                            tm *ltm = localtime(&now);
                            rec.mTime = Data(1900 + ltm->tm_year) + "-" +
                                        Data(1 + ltm->tm_mon) + "-"+
                                        Data(ltm->tm_mday) + " "+
                                        Data(ltm->tm_hour) + ":"+
                                        Data(ltm->tm_min) + ":" +
                                        Data(ltm->tm_sec);
                            mBase->addRoute(rec);
                            rec.mIdRoute = mBase->findRouteId(rec);
                            if (rec.mIdRoute >= 0)
                            {
                              routeList.push_back(rec);
                            }
                            else
                            {
                              send500(&sip);
                            }
                            //upd = true;
                        }
                      }
                      send200(&sip, *i);
                   }
        }
}

void
Registrar::removeAllContacts(resip::SipMessage* sip)
{
  //remove user contacts
  //find registrar information
   unsigned int idreg = findOrAddRegistrar(sip);
   if (0 != idreg)
    {
        int i = 0;
        for(RegDB::RouteRecord rec : routeList)
        {
          if (rec.mIdRegFk == idreg)
          {
             mBase->eraseRoute(rec.mIdRoute);
             routeList.erase(routeList.begin()+i);
           }
           i++;
        }
    }
}

bool
Registrar::testAuthorization(resip::SipMessage* sip)
{
  for (RegDB::AuthorizationRecord auth : authList)
  {
     if (Helper::authenticateRequestWithA1(*sip, mNameAddr, auth.mPassword, 0) == Helper::Authenticated)
        return true;
  }
  return false;
}

int
Registrar::findOrAddForward(resip::NameAddr& addr, unsigned int reg)
{
  unsigned int expires = reg;
  Data user = addr.uri().user();
  Data host = addr.uri().host();
  unsigned int port = addr.uri().port();
  Data scheme = addr.uri().scheme();

  unsigned int idProtocol = findProtocol(scheme);
  if (0 == idProtocol)
    idProtocol = addProtocol(scheme);
  if (0 == idProtocol)
  {
     ErrLog(<< "Not find Protocol");
     return 0;
  }

  unsigned int idDomain = findDomain(host);
  if (0 == idDomain)
     idDomain = addDomain(host);
  if (0 == idDomain)
  {
     ErrLog(<< "Not find Domain");
     return 0;
  }

  unsigned int idForward = findForward(idProtocol, idDomain, port);
  if (0 == idForward)
    idForward = addForward(idProtocol, idDomain, port);
  return idForward;
}

int
Registrar::findForward(const unsigned int& idProtocol,
                const unsigned int& idDomain,
                const unsigned int& port)
{
  unsigned int idForward = 0;
  for (RegDB::ForwardRecord rec : forwardList)
  {
     if ((idProtocol == rec.mIdProtocolFk) &&
         (port == rec.mPort) &&
         (idDomain == rec.mIdDomainFk))
     {
       idForward = rec.mIdForward;
       break;
     }
  }
  if (0 == idForward)
  {
    RegDB::ForwardRecord rec;
    rec.mIdProtocolFk = idProtocol;
    rec.mPort = port;
    rec.mIdDomainFk = idDomain;
    idForward = mBase->findForwardId(rec);
    if (idForward >= 0)
         forwardList.push_back(rec);
  }
  return idForward;
}

int
Registrar::addForward(const unsigned int& idProtocol,
                const unsigned int& idDomain,
                const unsigned int& port)
{
  RegDB::ForwardRecord rec;
  rec.mIdProtocolFk = idProtocol;
  rec.mIdDomainFk = idDomain;
  rec.mPort = port;
  mBase->addForward(rec);
  rec.mIdForward = mBase->findForwardId(rec);
  if (rec.mIdForward > 0)
       forwardList.push_back(rec);
  return rec.mIdForward;
}

int
Registrar::findProtocol(resip::Data& protocol)
{
  unsigned int idProtocol = 0;
  for (RegDB::ProtocolRecord rec : protocolList)
  {
     if (protocol == rec.mProtocol)
     {
       idProtocol = rec.mIdProtocol;
       break;
     }
  }
  if (0 == idProtocol)
  {
    RegDB::ProtocolRecord rec;
    rec.mProtocol = protocol;
    idProtocol = mBase->findProtocolId(rec);
    if (idProtocol > 0)
         protocolList.push_back(rec);
  }
  return idProtocol;
}

int
Registrar::addProtocol(resip::Data& protocol)
{
  RegDB::ProtocolRecord rec;
  rec.mProtocol = protocol;
  mBase->addProtocol(rec);
  rec.mIdProtocol = mBase->findProtocolId(rec);
  if (rec.mIdProtocol > 0)
       protocolList.push_back(rec);
  return rec.mIdProtocol;
}

int
Registrar::findDomain(resip::Data& host)
{
  unsigned int idDomain = 0;
  for (RegDB::DomainRecord rec : domainList)
  {
     if (host == rec.mDomain)
     {
       idDomain = rec.mIdDomain;
       break;
     }
  }
  if (0 == idDomain)
  {
    RegDB::DomainRecord rec;
    rec.mDomain = host;
    idDomain = mBase->findDomainId(rec);
    if (idDomain > 0)
         domainList.push_back(rec);
  }
  return idDomain;
}

int
Registrar::addDomain(resip::Data& host)
{
     RegDB::DomainRecord rec;
     rec.mDomain = host;
     mBase->addDomain(rec);
     rec.mIdDomain = mBase->findDomainId(rec);
     if (rec.mIdDomain > 0)
          domainList.push_back(rec);
     return rec.mIdDomain;
}

int
Registrar::findUser(resip::Data& usr)
{
  unsigned int idu = 0;
  for (RegDB::UserRecord rec : userList)
  {
     if (usr == rec.mName)
     {
       idu = rec.mIdUser;
       break;
     }
  }
  if (0 == idu)
  {
    RegDB::UserRecord rec;
    rec.mName = usr;
    idu = mBase->findUserId(rec);
    if (idu > 0)
         userList.push_back(rec);
  }
  return idu;
}

int
Registrar::addUser(resip::Data& usr)
{
  RegDB::UserRecord rec;
  rec.mName = usr;
  mBase->addUser(rec);
  rec.mIdUser = mBase->findUserId(rec);
  if (rec.mIdUser > 0)
       userList.push_back(rec);
  return rec.mIdUser;
}

int
Registrar::findUserDomain(int usr, int dom)
{
    int idud = 0;
    for (RegDB::UserDomainRecord rec : userDomeinList)
    {
        if ((usr == rec.mIdUserFk) && (rec.mIdDomainFk == dom))
        {
           idud = rec.mIdUserDomain;
           break;
         }
    }
    if (0 == idud)
    {
      RegDB::UserDomainRecord rec;
      rec.mIdUserFk = usr;
      rec.mIdDomainFk = dom;
      idud = mBase->findUserDomainId(rec);
      if (idud > 0)
           userDomeinList.push_back(rec);
    }
    return idud;
}

int
Registrar::addUserDomain(int usr, int dom)
{
  RegDB::UserDomainRecord rec;
  rec.mIdUserFk = usr;
  rec.mIdDomainFk = dom;
  mBase->addUserDomain(rec);
  rec.mIdUserDomain = mBase->findUserDomainId(rec);
  if (rec.mIdUserDomain > 0)
       userDomeinList.push_back(rec);
  return rec.mIdUserDomain;
}

int
Registrar::findOrAddRegistrar(resip::SipMessage* sip)
{
  NameAddr& to = sip->header(h_To);
  NameAddr& from = sip->header(h_From);
  Data& callid = sip->header(h_CallId).value();

  Data fromUserName = from.uri().user();
  Data fromUserHost = from.uri().host();
  Data toUserName = to.uri().user();
  Data toUserHost = to.uri().host();
  unsigned int fromIdDomain = 0, toIdDomain = 0;
  unsigned int fromIdUser = 0, toIdUser = 0;
  unsigned int fromIdUserDomain = 0, toIdUserDomain = 0;
  bool equal = false;

  if ((fromUserName == toUserName) && (toUserHost == fromUserHost))
    equal = true;
  //find id domain
  fromIdDomain = findDomain(fromUserHost);
  if (equal)
     toIdDomain = fromIdDomain;
  else
     toIdDomain = findDomain(toUserHost);
  //find id user
  fromIdUser = findUser(fromUserName);
  if (0 == fromIdUser)
     fromIdUser = addUser(fromUserName);

  if (equal)
     toIdUser = fromIdUser;
  else
  {
     toIdUser = findUser(toUserName);
     if (0 == toIdUser)
        toIdUser = addUser(toUserName);
   }
  //find id user-domain
  fromIdUserDomain = findUserDomain(fromIdUser, fromIdDomain);
  if (0 == fromIdUserDomain)
      fromIdUserDomain = addUserDomain(fromIdUser, fromIdDomain);

  if (equal)
     toIdUserDomain = fromIdUserDomain;
  else
  {
     toIdUserDomain = findUserDomain(toIdUser, toIdDomain);
     if (0 == toIdUserDomain)
         toIdUserDomain = addUserDomain(toIdUser, toIdDomain);
   }

  if ((0 == fromIdUser) ||
      (0 == toIdUser) ||
      (0 == fromIdDomain) ||
      (0 == toIdDomain) ||
      (0 == toIdUserDomain) ||
      (0 == fromIdUserDomain) )
      {
        return 0;
      }
  //find registrar
  unsigned int idreg = findRegistrar(toIdUserDomain, fromIdUserDomain, callid);
  if (equal && 0 == idreg)
       idreg  = addRegistrar(toIdUserDomain, fromIdUserDomain, callid);

  if (0 == idreg)
  {
    ErrLog(<< "No registrat record");
  }
  return idreg;
}

int
Registrar::findRegistrar(const unsigned int& to,
                const unsigned int& from,
                resip::Data& callid)
{
  unsigned int idreg = 0;
  for (RegDB::RegistrarRecord rec : regList)
  {
    if ((from == rec.mIdMainFk) &&
        (to == rec.mIdUserDomainFk))
        {
           if(callid != rec.mCallId)
           {
             rec.mCallId = callid;
             mBase->updateRegistrar(rec.mIdReg,rec);
           }
           idreg = rec.mIdReg;
           break;
      }
  }
  if (0 == idreg)
  {
    RegDB::RegistrarRecord rec;
    rec.mIdMainFk = from;
    rec.mIdUserDomainFk = to;
    rec.mCallId = callid;
    idreg = mBase->findRegistrarId(rec);
    if (idreg > 0)
         regList.push_back(rec);
  }
  return idreg;
}

int
Registrar::addRegistrar(const unsigned int& to,
                const unsigned int& from,
                resip::Data& callid)
{
  RegDB::RegistrarRecord rec;
  rec.mIdUserDomainFk = to;
  rec.mIdMainFk = from;
  rec.mCallId = callid;
  mBase->addRegistrar(rec);
  rec.mIdReg = mBase->findRegistrarId(rec);
  if (rec.mIdReg > 0)
       regList.push_back(rec);
  return rec.mIdReg;
}

/******************************************************************************/
/*                         MESSAGES                                           */
/******************************************************************************/
void
Registrar::send200(SipMessage* sip, NameAddr add)
{
  auto_ptr<SipMessage> msg200(Helper::makeResponse(*sip, 200, add));
  InfoLog(<< "Sent 200 to REGISTER");
  std::lock_guard<std::mutex> lk(mut);
  mStack.send(*msg200);
}

void
Registrar::send400(SipMessage* sip){
  auto_ptr<SipMessage> msg400(Helper::makeResponse(*sip, 400));
  ErrLog (<< "Sent 400(Bad Request) to REGISTER");
  std::lock_guard<std::mutex> lk(mut);
  mStack.send(*msg400);
}

void
Registrar::send401(SipMessage* sip){
  auto_ptr<SipMessage> msg401(Helper::makeWWWChallenge(*sip, mNameAddr, true, false));
  ErrLog (<< "Sent 401(Unauthorized) to REGISTER");
  std::lock_guard<std::mutex> lk(mut);
  mStack.send(*msg401);
}

void
Registrar::send405(SipMessage* sip, Data meth)
{
  int IMMethodList[] = {(int) REGISTER };
  const int IMMethodListSize = sizeof(IMMethodList) / sizeof(*IMMethodList);
  auto_ptr<SipMessage> msg405(Helper::make405(*sip, IMMethodList, IMMethodListSize));
  ErrLog(<< "Sent 405(Method Not Allowed) to "<< meth);
  std::lock_guard<std::mutex> lk(mut);
  mStack.send(*msg405);
}


void
Registrar::send403(SipMessage* sip, Data mes)
{
  auto_ptr<SipMessage> msg403(Helper::makeResponse(*sip, 403));
  ErrLog(<< "Sent 403(Forbidded) to REGISTER :" + mes);
  std::lock_guard<std::mutex> lk(mut);
  mStack.send(*msg403);
}

void
Registrar::send500(SipMessage* sip){
  auto_ptr<SipMessage> msg500(Helper::makeResponse(*sip, 500));
  ErrLog (<< "Sent 500(Server Internal Error) to REGISTER");
  std::lock_guard<std::mutex> lk(mut);
  mStack.send(*msg500);
}
