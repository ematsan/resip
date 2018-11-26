//./server debug 1 tcp

#ifndef WIN32
#include <sys/time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include <sys/types.h>
#include <iostream>
#include <memory>

#include "resip/stack/Helper.hxx"
#include "resip/stack/SipMessage.hxx"
#include "resip/stack/Uri.hxx"
#include "resip/stack/Auth.hxx"
#include "resip/stack/SipStack.hxx"
#include "rutil/Logger.hxx"
#include "rutil/ThreadIf.hxx"

using namespace resip;
using namespace std;

#define RESIPROCATE_SUBSYSTEM Subsystem::TEST

class Server : public ThreadIf
{
    public:

      Server(SipStack& stack, TransportType transport, NameAddr contact)
         : mStack(stack),
           mNumCalls(0),
           mTransport(transport),
           mNameAddr(contact)
      {}

      void thread()
      {
         InfoLog(<<"This is the Server");

         UInt64 startTime = Timer::getTimeMs();

         while(true)
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
               ErrLog ( << "Server received: " << getMethodName(meth));
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
                   auto_ptr<SipMessage> msg401(Helper::makeResponse(*received, 401, mNameAddr));
                   ErrLog( << "Sent 401(Unauthorized) to REGISTER");
                   mStack.send(*msg401);
                }

               mNumCalls++;
               auto_ptr<SipMessage> msg200(Helper::makeResponse(*received, 200, mNameAddr));
               ErrLog( << "Sent 200 to REGISTER");
               mStack.send(*msg200);

               /*if ( meth == REGISTER )
               {
                  Data localTag = Helper::computeTag(4);
                  auto_ptr<SipMessage> msg180(Helper::makeResponse(*received, 180, contact));
                  msg180->header(h_To).param(p_tag) = localTag;
                  ErrLog( << "Sent 180");
                  mStack.send( *msg180);

                  auto_ptr<SipMessage> msg200(Helper::makeResponse(*received, 200, contact));
                  msg200->header(h_To).param(p_tag) = localTag;
                  ErrLog( << "Sent 200");
                  mStack.send(*msg200);
               }
               if ( meth == BYE)
               {
                  auto_ptr<SipMessage> msg200(Helper::makeResponse(*received, 200, contact));
                  ErrLog( << "Sent 200 to BYE");
                  mStack.send(*msg200);
               }*/
               UInt64 endTime = Timer::getTimeMs();

               CritLog(<< "Completed: " << mNumCalls << " calls in " << endTime - startTime << "ms, "
                       << mNumCalls*1000 / (float)(endTime - startTime) << " CPS");
              }
            }
         }
      }
   private:
      SipStack& mStack;
      int mNumCalls;
      TransportType mTransport;
      NameAddr mNameAddr;
};

int
main(int argc, char* argv[])
{
   Log::initialize(Log::Cout, Log::toLevel("debug"), argv[0]);
//   Log::initialize(Log::Cout, Log::toLevel(argv[1]), argv[0]);
   SipStack stack;
   //read preference


   //connect mySQL

   //read protocol and port from property
   TransportType protocol;
   int port;
   //protocol = UDP;
   protocol = TCP;
   port = 5070;
   stack.addTransport(protocol, port);
   //read contact
   NameAddr contact;
   contact.uri().scheme() = "sip";
   contact.uri().user() = "fluffy";
   contact.uri().host() = SipStack::getHostname();
   contact.uri().port() = port;
   contact.uri().param(p_transport) = Tuple::toData(protocol);

   ::Server server(stack, protocol, contact);

   server.run();
   server.join();
   return 0;
}
