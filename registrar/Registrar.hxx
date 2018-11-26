#if !defined(REGISTRAR_HXX)
#define REGISTRAR_HXX

#include "resip/dum/RegistrationHandler.hxx"
#include "resip/dum/DialogUsageManager.hxx"
#include "resip/dum/InMemoryRegistrationDatabase.hxx"
#include "resip/dum/MasterProfile.hxx"

namespace registrar
{
//class Proxy;

class Registrar: public resip::ServerRegistrationHandler
{
   public:
      Registrar();
      virtual ~Registrar();

      void setProxy(Proxy* proxy) { mProxy = proxy; }

      virtual void addRegistrarHandler(RegistrarHandler* handler);

      virtual void onRefresh(resip::ServerRegistrationHandle, const resip::SipMessage& reg);
      virtual void onRemove(resip::ServerRegistrationHandle, const resip::SipMessage& reg);
      virtual void onRemoveAll(resip::ServerRegistrationHandle, const resip::SipMessage& reg);
      virtual void onAdd(resip::ServerRegistrationHandle, const resip::SipMessage& reg);
      virtual void onQuery(resip::ServerRegistrationHandle, const resip::SipMessage& reg);

   private:
      std::list<RegistrarHandler*> mRegistrarHandlers;
      Proxy* mProxy;
};
}
#endif
