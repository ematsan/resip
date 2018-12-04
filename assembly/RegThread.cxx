#include <iostream>
#include "RegThread.hxx"
#include "RegMySQL.hxx"
#include "RegDB.hxx"
//#include "RegRunner.hxx"

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

RegThread::RegThread(SipStack& stack, Data realm, RegMySQL* mdatabase)
   : mStack(stack)
   , mNameAddr(realm)
   , mBase(mdatabase)
{
  cout<<"RegThread constructor"<<endl;
  //select all data
  loadData();
}


RegThread::~RegThread()
{
   cout<<"RegThread destructor"<<endl;
}

void
RegThread::loadData()
{
  ulist = mBase->getAllUsers();
  if (ulist.empty()){ ErrLog(<< "No element in table tUser"); }
  dlist = mBase->getAllDomains();
  if (dlist.empty()) { ErrLog(<< "No element in table tDomain"); }
  flist = mBase->getAllForwards();
  if (flist.empty()){ ErrLog(<< "No element in table tForward"); }
  plist = mBase->getAllProtocols();
  if (plist.empty())  { ErrLog(<< "No element in table tProtocol"); }
  alist = mBase->getAllAuthorizations();
  if (alist.empty()) { ErrLog(<< "No element in table tAuthorization");}
  reglist = mBase->getAllRegistrars();
  if (reglist.empty()) { ErrLog(<< "No element in table tRegistrar"); }
  rlist = mBase->getAllRoutes();
  if (rlist.empty()) { ErrLog(<< "No element in table tRoute"); }
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
        auto_ptr<SipMessage> forDel(received);
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
  }
}

void
RegThread::analisysRequest(resip::SipMessage* sip)
{
  Auths& auth =  sip->header(h_Authorizations);
  //test Authorization
  if (!testAuthorization(sip))
  {
     send400(sip);
     return;
   }
  //test Registrar
  unsigned int idreg = findRegistrar(sip);
  if (0 == idreg)
  {
     send400(sip);
     return;
   }

  if(sip->exists(h_Contacts))
        {

           NameAddr& to = sip->header(h_To);
           NameAddr& from = sip->header(h_From);
           CallId& callid = sip->header(h_CallId);

           Data fuser = from.uri().user();
           Data fhost = from.uri().host();
           Data tuser = to.uri().user();
           Data thost = to.uri().host();


           //cout<<"\n\n\n\n\n"<<callid<<"\n\n\n\n\n";
           /*for(RegDB::AuthorizationRecord auth: alist)
           {
             cout<<"\n\n\n\n\n"<<auth.mIdDomain<<"\n\n\n\n\n";
           }*/
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
                       return;
                     }
                     // Check for "Contact: *" style deregistration
                      if (i->isAllContacts())
                      {
                        if (contacts.size() > 1 || expires != 0)
                          {
                              send400(sip);
                              return;
                           }
                        //remove users contacts
                        removeAllContacts(sip);
                        return;
                      }

                      NameAddr addr = *i;

                      /*Data user = addr.uri().user();
                      Data host = addr.uri().host();
                      unsigned int port = addr.uri().port();
                      Data scheme = addr.uri().scheme();
                      cout<<user<<" - "<< host <<" - "<< port <<" - " << scheme <<"\n\n\n\n\n\n\n\n";*/
                      if (addr.exists(p_expires))
                         expires = addr.param(p_expires);

                      unsigned int idf = findForward(addr, expires);
                      if (0 == idf)
                      {
                         send400(sip);
                         return;
                       }

                      //need to delete record
                      if (0 == expires)
                      {
                         /*******************************************/

                      }
                      //to.param(p_tag)
                      //cout<<"\n\n\n\n\n"<<from.param(p_tag)<<"\n\n\n\n\n";
                      //cout<<"\n\n\n\n\n"<<host<<"\n\n\n\n\n";
                      send200(sip, *i);
                   }
        }
        else //no contacts
        {
              //do nothing
              //send400(sip);
        }
}

void
RegThread::removeAllContacts(resip::SipMessage* sip)
{
  //remove user contacts
  //find registrar information
   unsigned int idreg = findRegistrar(sip);
   if (0 != idreg)
    {
        bool del = false;
        for(RegDB::RouteRecord rec : rlist)
        {
          if (rec.mIdReg == idreg)
          {
             mBase->eraseRoute(Data(rec.mIdRoute));
             del = true;
           }
        }
        //if remove then reload data
        if (del)
        {
          rlist.clear();
          rlist = mBase->getAllRoutes();
        }
    }
}

bool
RegThread::testAuthorization(resip::SipMessage* sip)
{
  for (RegDB::AuthorizationRecord auth : alist)
  {
     //cout<< auth.mIdAuth<< " - "<<auth.mPassword<<"\n\n\n\n\n";
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
  //cout<<user<<" - "<< host <<" - "<< port <<" - " << scheme <<"\n\n\n\n\n\n\n\n";
  unsigned int idp = findProtocol(scheme);
  if (0 == idp)
  {
     return 0;
  }

  unsigned int idd = findDomain(host);
  if (0 == idd)
  {
     return 0;
  }

  unsigned int idf = findForward(idp, idd, host, port);
  if (0 == idf)
  {
     return idf;
  }

  return idf;
}

int
RegThread::findForward(const unsigned int& idp,
                const unsigned int& idd,
                const resip::Data& ip = "127.0.0.1",
                const unsigned int& port = 0)
{
  //ip and port do not use
  //wait nat
  unsigned int idf = 0;
  for (RegDB::ForwardRecord rec : flist)
  {
     if ((idp == rec.mIdProtocol) && (idd == rec.mIdDomain))
     {
       idf = rec.mIdForward;
       break;
     }
  }
  //add forward
  if (0 == idp)
  {
     RegDB::ForwardRecord rec;
     rec.mIdProtocol = idp;
     rec.mIdDomain = idd;
     //ip and port do not use
     //wait nat
     mBase->addForward(Data(rec.mIdForward), rec);
     //reload
     flist.clear();
     flist = mBase->getAllForwards();
     idf = flist.back().mIdForward;
  }
  return idf;
}


int
RegThread::findProtocol(resip::Data& protocol)
{
  unsigned int idp = 0;
  for (RegDB::ProtocolRecord rec : plist)
  {
     if (protocol == rec.mProtocol)
     {
       idp = rec.mIdProtocol;
       break;
     }
  }
  //add protocol
  if (0 == idp)
  {
     RegDB::ProtocolRecord rec;
     rec.mProtocol = protocol;
     mBase->addProtocol(Data(rec.mIdProtocol), rec);
     //reload
     plist.clear();
     plist = mBase->getAllProtocols();
     idp = plist.back().mIdProtocol;
  }
  return idp;
}

int
RegThread::findDomain(resip::Data& host)
{
  unsigned int idd = 0;
  for (RegDB::DomainRecord rec : dlist)
  {
     if (host == rec.mDomain)
     {
       idd = rec.mIdDomain;
       break;
     }
  }
  //add domain
  if (0 == idd)
  {
     RegDB::DomainRecord rec;
     rec.mDomain = host;
     mBase->addDomain(Data(rec.mIdDomain), rec);
     //reload
     dlist.clear();
     dlist = mBase->getAllDomains();
     idd = dlist.back().mIdDomain;
  }
  return idd;
}

int
RegThread::findRegistrar(resip::SipMessage* sip)
{
  NameAddr& to = sip->header(h_To);
  NameAddr& from = sip->header(h_From);
  //CallId&
  Data& callid = sip->header(h_CallId).value();

  Data fuser = from.uri().user();
  Data fhost = from.uri().host();
  Data tuser = to.uri().user();
  Data thost = to.uri().host();
  unsigned int fidd = 0, tidd = 0;
  unsigned int fidu = 0, tidu = 0;
  bool equal = false;
//  cout << to.uri() << "\n\n\n\n\n";
//  cout << from.uri() << "\n\n\n\n\n";
  //if (to.uri() == from.uri())
  if ((fuser == tuser) && (thost == fhost))
  {
    equal = true;
  }
  //find id domain
  for (RegDB::DomainRecord rec : dlist)
  {
      if (fhost == rec.mDomain)
         fidd = rec.mIdDomain;
      if (thost == rec.mDomain)
          tidd = rec.mIdDomain;
      if ((0 != tidd) && (0 != fidd))
         break;
  }
  //find id user
  for (RegDB::UserRecord rec : ulist)
  {
      if ((fuser == rec.mName) && (rec.mIdDomain == fidd))
      {
         fidu = rec.mIdUser;
       }
      if ((tuser == rec.mName) && (rec.mIdDomain == tidd))
      {
          tidu = rec.mIdUser;
        }
      if ((0 != tidu) && (0 != fidu))
      {
         break;
       }
  }
  //cout << fidu << "\t"<< tidu << "\t"<< fidd << "\t"<< tidd << "\n\n\n";
  if ((0 == fidu) ||
      (0 == tidu) ||
      (0 == fidd) ||
      (0 == tidd))
      {
        return 0;
      }
  //find registrar
  unsigned int idreg = 0;
  for (RegDB::RegistrarRecord rec : reglist)
  {
    //-------------------------------------------------------------//
    //-------------------Test CallId-------------------------------//
    //-------------------------------------------------------------//
    if (//(callid == rec.mCallId) &&
        (fidu == rec.mIdMain) &&
        (tidu == rec.mIdUser) &&
        (tidd == rec.mIdDomain))
        {
           idreg = rec.mIdReg;
           break;
      }
  }
  //add registrar
  if ((idreg == 0) && (equal))
  {
      RegDB::RegistrarRecord rec;
      rec.mIdUser = fidu;
      rec.mIdMain = fidu;
      rec.mCallId = callid;
      rec.mIdDomain = tidd;
      rec.mIdReg = 0;
      RegDB::Key key;
      mBase->addRegistrar(key, rec);
      //reload
      reglist.clear();
      reglist = mBase->getAllRegistrars();
      idreg = reglist.back().mIdReg;
      InfoLog(<< "Add registrar");
  }
  if (0 == idreg)
  {
    ErrLog(<< "No registrat record\n\n\n\n\n");
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
  //auto_ptr<SipMessage> msg401(Helper::makeResponse(*received, 401, mNameAddr));
  auto_ptr<SipMessage> msg401(Helper::makeWWWChallenge(*sip, mNameAddr/*"localhost"*/, true, false));
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
  ErrLog(<< "Sent 405 to "<< meth);
}


void
RegThread::send403(SipMessage* sip, Data meth)
{
  auto_ptr<SipMessage> msg403(Helper::makeResponse(*sip, 403));
  mStack.send(*msg403);
  ErrLog(<< "Sent 403 to "<< meth);
}
