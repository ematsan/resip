#include <iostream>
#include <ctime>
#include "RegThread.hxx"
#include "RegMySQL.hxx"
#include "RegDB.hxx"

//!!!!!!!!!!!!!!!!!!!!!! - read example using messages
#include "resip/stack/Helper.hxx"
//!!!!!!!!!!!!!!!!!!!!!! - read example using header
#include "resip/stack/SipMessage.hxx"
//!!!!!!!!!!!! resip/stack/NameAddr.hxx - read example using parametrs address
#include "resip/stack/Uri.hxx"
//!!!!!!!!!!!!read example using parametrs auth
#include "resip/stack/Auth.hxx"
#include "resip/stack/SipStack.hxx"
#include "rutil/Logger.hxx"

using namespace resip;
using namespace std;
using namespace registrar;

//RegThread::RegThread(SipStack& stack, Data realm, RegMySQL* mdatabase, const vector<Data>& configDomains)
RegThread::RegThread(SipStack& stack, Data realm, RegDB* mdatabase, const vector<Data>& configDomains)
   : mStack(stack)
   , mNameAddr(realm)
   , mBase(mdatabase)
   , mConfigDomains(configDomains)
{
  InfoLog(<<"RegThread constructor");
  //select all data
  loadData();
  reloadDomain();

}

RegThread::~RegThread()
{
   InfoLog(<<"RegThread destructor");
   clearData();
}

void
RegThread::reloadDomain()
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
        n.mIdDomain = mBase->findDomain(n);
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
RegThread::clearData()
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
RegThread::loadData()
{
  clearData();
  userList = mBase->getAllUsers();
  if (userList.empty()){ ErrLog(<< "No element in table toUserName"); }
  domainList = mBase->getAllDomains();
  if (domainList.empty()) { ErrLog(<< "No element in table tDomain"); }
  userDomeinList = mBase->getAllUserDomains();
  if (userDomeinList.empty()) { ErrLog(<< "No element in table toUserNameDomain"); }
  forwardList = mBase->getAllForwards();
  if (forwardList.empty()){ ErrLog(<< "No element in table tForward"); }
  protocolList = mBase->getAllProtocols();
  if (protocolList.empty())  { ErrLog(<< "No element in table tProtocol"); }
  authList = mBase->getAllAuthorizations();
  if (authList.empty()) { ErrLog(<< "No element in table tAuthorization");}
  regList = mBase->getAllRegistrars();
  if (regList.empty()) { ErrLog(<< "No element in table tRegistrar"); }
  routeList = mBase->getAllRoutes();
  if (routeList.empty()) { ErrLog(<< "No element in table tRoute"); }
}

void
RegThread::thread()
{
  InfoLog(<<"This is the Server");
  while (!isShutdown())
  {
     FdSet fdset;
     mStack.buildFdSet(fdset);
     int err = fdset.selectMilliSeconds(0);
     assert (err != -1);
     mStack.process(fdset);
     SipMessage* received = mStack.receive();
     if (received)
     {
        MethodTypes meth = received->header(h_RequestLine).getMethod();
        InfoLog(<< "Server received: " << getMethodName(meth));
        if ( meth == REGISTER )
        {
          //NameAddrs& contacts = received->header(h_Contacts);
          //ExpiresCategory& expires = received->header(h_Expires);
          if (received->exists(h_Authorizations))
           {
              analisysRequest(received);
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
RegThread::analisysRequest(resip::SipMessage* sip)
{
  Auths& auth =  sip->header(h_Authorizations);
  //test Authorization
  if (!testAuthorization(sip))
  {
     send403(sip, "User not register");
     return;
   }
  //test Registrar
  unsigned int idreg = findRegistrar(sip);
  if (0 == idreg)
  {
     send403(sip, "User not have access to add record");
     return;
   }

  if(sip->exists(h_Contacts))
        {
           NameAddr& to = sip->header(h_To);
           NameAddr& from = sip->header(h_From);
           CallId& callid = sip->header(h_CallId);

           Data fromUserName = from.uri().user();
           Data fromUserHost = from.uri().host();
           Data toUserName = to.uri().user();
           Data toUserHost = to.uri().host();
           //registration time
           unsigned int expires = 0;
           if (sip->exists(h_Expires))
           {
             expires = sip->header(h_Expires).value();
           }
           else
           {
             expires = 3600;
           }
           //parse all contacts
           ParserContainer<NameAddr>& contacts = sip->header(h_Contacts);

           for (ParserContainer<NameAddr>::iterator i = contacts.begin(); i != contacts.end(); i++)
                   {
                     //test format
                     if (!i->isWellFormed())
                     {
                       send400(sip);
                       ErrLog(<< "Not well formed");
                       return;
                     }
                     // Check for "Contact: *" style deregistration
                      if (i->isAllContacts())
                      {
                        if (contacts.size() > 1 || expires != 0)
                          {
                              send400(sip);
                              ErrLog(<< "Error us Contact:*");
                              return;
                           }
                        //remove users contacts
                        removeAllContacts(sip);
                        return;
                      }

                      NameAddr addr = *i;

                      if (addr.exists(p_expires))
                         expires = addr.param(p_expires);

                      unsigned int idForward = findForward(addr, expires);
                      if (0 == idForward)
                      {
                         send500(sip);
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
                             mBase->eraseRoute(Data(rec.mIdRoute));
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
                             //https://www.tutorialspoint.com/cplusplus/cpp_date_time.htm
                             tm *ltm = localtime(&now);
                             // print various components of tm structure.
                             //http://mycpp.ru/cpp/scpp/cppd_datetime.html
                             rec.mTime = Data(1900 + ltm->tm_year) + "-" +
                                         Data(1 + ltm->tm_mon) + "-" +
                                         Data(ltm->tm_mday) + " "+
                                         Data(ltm->tm_hour) + ":"+
                                         Data(ltm->tm_min) + ":" +
                                         Data(ltm->tm_sec);
                             mBase->updateRoute(Data(rec.mIdRoute), rec);
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
                            rec.mIdRoute = mBase->findRoute(rec);
                            if (rec.mIdRoute >= 0)
                            {
                              routeList.push_back(rec);
                            }
                            else
                            {
                              send500(sip);
                            }
                            //upd = true;
                        }
                      }
                      send200(sip, *i);
                   }
        }
        else ;//no contacts - do nothing
}

void
RegThread::removeAllContacts(resip::SipMessage* sip)
{
  //remove user contacts
  //find registrar information
   unsigned int idreg = findRegistrar(sip);
   if (0 != idreg)
    {
        int i = 0;
        for(RegDB::RouteRecord rec : routeList)
        {
          if (rec.mIdRegFk == idreg)
          {
             mBase->eraseRoute(Data(rec.mIdRoute));
             routeList.erase(routeList.begin()+i);
           }
           i++;
        }
    }
}

bool
RegThread::testAuthorization(resip::SipMessage* sip)
{
  for (RegDB::AuthorizationRecord auth : authList)
  {
     if (Helper::authenticateRequestWithA1(*sip, mNameAddr, auth.mPassword, 0) == Helper::Authenticated)
        return true;
  }
  return false;
}

int
RegThread::findForward(resip::NameAddr& addr, unsigned int reg)
{
  unsigned int expires = reg;
  Data user = addr.uri().user();
  Data host = addr.uri().host();
  unsigned int port = addr.uri().port();
  Data scheme = addr.uri().scheme();

  unsigned int idProtocol = findProtocol(scheme, true);
  if (0 == idProtocol)
  {
     ErrLog(<< "Not find Protocol");
     return 0;
  }

  unsigned int idDomain = findDomain(host, true);
  if (0 == idDomain)
  {
     ErrLog(<< "Not find Domain");
     return 0;
  }

  unsigned int idForward = findForward(idProtocol, idDomain, port, true);
  return idForward;
}

int
RegThread::findForward(const unsigned int& idProtocol,
                const unsigned int& idDomain,
                const unsigned int& port, bool upd)
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
  //add forward
  if ((0 == idForward) && (upd))
  {
     RegDB::ForwardRecord rec;
     rec.mIdProtocolFk = idProtocol;
     rec.mIdDomainFk = idDomain;
     rec.mPort = port;
     mBase->addForward(rec);
     rec.mIdForward = mBase->findForward(rec);
     if (rec.mIdForward >= 0)
          forwardList.push_back(rec);
     idForward = rec.mIdForward;
  }
  return idForward;
}

int
RegThread::findProtocol(resip::Data& protocol, bool upd)
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
  //add protocol
  if ((0 == idProtocol) && (upd))
  {
     RegDB::ProtocolRecord rec;
     rec.mProtocol = protocol;
     mBase->addProtocol(rec);
     rec.mIdProtocol = mBase->findProtocol(rec);
     if (rec.mIdProtocol >= 0)
          protocolList.push_back(rec);
     idProtocol = rec.mIdProtocol;
  }
  return idProtocol;
}

int
RegThread::findDomain(resip::Data& host, bool upd)
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
  //add domain
  if ((0 == idDomain) && (upd))
  {
     RegDB::DomainRecord rec;
     rec.mDomain = host;
     mBase->addDomain(rec);
     rec.mIdDomain = mBase->findDomain(rec);
     if (rec.mIdDomain >= 0)
          domainList.push_back(rec);
     idDomain = rec.mIdDomain;
  }
  return idDomain;
}

int
RegThread::findUser(resip::Data& usr, bool upd)
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
  //add user
  if ((0 == idu) && (upd))
  {
     RegDB::UserRecord rec;
     rec.mName = usr;
     mBase->addUser(rec);
     rec.mIdUser = mBase->findUser(rec);
     if (rec.mIdUser >= 0)
          userList.push_back(rec);
     idu = rec.mIdUser;
  }
  return idu;
}

int
RegThread::findUserDomain(int usr, int dom, bool upd)
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
    //add userdomain
    if ((0 == idud) && (upd))
    {
      RegDB::UserDomainRecord rec;
      rec.mIdUserFk = usr;
      rec.mIdDomainFk = dom;
      mBase->addUserDomain(rec);
      rec.mIdUserDomain = mBase->findUserDomain(rec);
      if (rec.mIdUserDomain >= 0)
           userDomeinList.push_back(rec);
      idud = rec.mIdUserDomain;
    }
    return idud;
}

int
RegThread::findRegistrar(resip::SipMessage* sip)
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
  fromIdDomain = findDomain(fromUserHost, false);
  if (equal)
     toIdDomain = fromIdDomain;
  else
     toIdDomain = findDomain(toUserHost, false);
  //find id user
  fromIdUser = findUser(fromUserName, equal);
  if (equal)
     toIdUser = fromIdUser;
  else
     toIdUser = findUser(toUserName, equal);
  //find id user-domain
  fromIdUserDomain = findUserDomain(fromIdUser, fromIdDomain, equal);
  if (equal)
     toIdUserDomain = fromIdUserDomain;
  else
     toIdUserDomain = findUserDomain(toIdUser, toIdDomain, equal);

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
  unsigned int idreg = findRegistrar(toIdUserDomain, fromIdUserDomain, callid, equal);
  if (0 == idreg)
  {
    ErrLog(<< "No registrat record");
  }
  return idreg;
}

int
RegThread::findRegistrar(const unsigned int& to,
                const unsigned int& from,
                resip::Data& callid, bool upd)
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
             mBase->updateRegistrar(Data(rec.mIdReg),rec);
           }
           idreg = rec.mIdReg;
           break;
      }
  }
  //add registrar
  if ((idreg == 0) && (upd))
  {
      RegDB::RegistrarRecord rec;
      rec.mIdUserDomainFk = to;
      rec.mIdMainFk = from;
      rec.mCallId = callid;
      mBase->addRegistrar(rec);
      rec.mIdReg = mBase->findRegistrar(rec);
      if (rec.mIdReg >= 0)
           regList.push_back(rec);
      idreg = rec.mIdReg;
      InfoLog(<< "Add registrar");
  }
  return idreg;
}

/******************************************************************************/
/*                         MESSAGES                                           */
/******************************************************************************/
void
RegThread::send200(SipMessage* sip, NameAddr add)
{
  auto_ptr<SipMessage> msg200(Helper::makeResponse(*sip, 200, add));
  mStack.send(*msg200);
  InfoLog(<< "Sent 200 to REGISTER");
}

void
RegThread::send400(SipMessage* sip){
  auto_ptr<SipMessage> msg400(Helper::makeResponse(*sip, 400));
  ErrLog (<< "Sent 400(Bad Request) to REGISTER");
  mStack.send(*msg400);
}

void
RegThread::send401(SipMessage* sip){
  auto_ptr<SipMessage> msg401(Helper::makeWWWChallenge(*sip, mNameAddr, true, false));
  ErrLog (<< "Sent 401(Unauthorized) to REGISTER");
  mStack.send(*msg401);
}

void
RegThread::send405(SipMessage* sip, Data meth)
{
  int IMMethodList[] = {(int) REGISTER };
  const int IMMethodListSize = sizeof(IMMethodList) / sizeof(*IMMethodList);
  auto_ptr<SipMessage> msg405(Helper::make405(*sip, IMMethodList, IMMethodListSize));
  mStack.send(*msg405);
  ErrLog(<< "Sent 405(Method Not Allowed) to "<< meth);
}


void
RegThread::send403(SipMessage* sip, Data mes)
{
  //auto_ptr<SipMessage> msg403(Helper::makeResponse(*sip, 403, mes));
  auto_ptr<SipMessage> msg403(Helper::makeResponse(*sip, 403));
  mStack.send(*msg403);
  ErrLog(<< "Sent 403(Forbidded) to REGISTER :" + mes);
}

void
RegThread::send500(SipMessage* sip){
  auto_ptr<SipMessage> msg500(Helper::makeResponse(*sip, 500));
  ErrLog (<< "Sent 500(Server Internal Error) to REGISTER");
  mStack.send(*msg500);
}
