#include <iostream>
#include "RegThread.hxx"
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
//#include "rutil/Logger.hxx"


using namespace resip;
using namespace std;
using namespace registrar;

RegThread:: RegThread(SipStack& stack, NameAddr contact)
   : mStack(stack),
     mNameAddr(contact)
{
  cout<<"RegThread constructor"<<endl;
}


RegThread::~RegThread()
{
   cout<<"RegThread destructor"<<endl;
}

void
RegThread::thread()
{
  cout<<"This is the Server";
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
        cerr << "Server received: " << getMethodName(meth)<<endl;

//!!!!
        if ( meth == REGISTER )
        {
          NameAddr& to = received->header(h_To);
          NameAddr& from = received->header(h_From);
          NameAddrs& contacts = received->header(h_Contacts);
          ExpiresCategory& expires = received->header(h_Expires);

          if (received->exists(h_Authorizations)){
            Auths& auth =  received->header(h_Authorizations);
            //test authorization
          }
          else{
            //auto_ptr<SipMessage> msg401(Helper::makeResponse(*received, 401, mNameAddr));
            auto_ptr<SipMessage> msg401(Helper::makeWWWChallenge(*received, "localhost", true,false));
            cerr << "Sent 401(Unauthorized) to REGISTER"<<endl;
            mStack.send(*msg401);
         }

        auto_ptr<SipMessage> msg200(Helper::makeResponse(*received, 200, mNameAddr));
        cout << "Sent 200 to REGISTER" << endl;
        mStack.send(*msg200);
       }
     }
  }

}
