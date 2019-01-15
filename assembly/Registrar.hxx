#if !defined(REGISTRAR_HXX)
#define REGISTRAR_HXX

#include <vector>
#include <thread>
#include <atomic>
#include <queue>
#include <mutex>
#include <functional>

#include "rutil/ThreadIf.hxx"
#include "resip/stack/NameAddr.hxx"

#include "RegDB.hxx"

namespace resip
{
  class SipStack;
  class Data;
  class SipMessage;
}
//work with data
namespace registrar
{
class RegMySQL;

class join_threads
{
  std::vector<std::thread>& threads;
public:
  explicit join_threads(std::vector<std::thread>& threads_): threads(threads_){}
  ~join_threads()
  {
    for(unsigned long i = 0; i < threads.size(); ++i)
    {
      if (threads[i].joinable())
         threads[i].join();
    }
  }
};

template<typename T>
class thread_safe_queue
{
  mutable std::mutex mut;
  std::queue<T> data_queue;
public:
  thread_safe_queue(){}

  void push(T new_value)
  {
    std::lock_guard<std::mutex> lk(mut);
    data_queue.push(std::move(new_value));
  }

  bool try_pop(T& value)
  {
    std::lock_guard<std::mutex> lk(mut);
    if (data_queue.empty())
        return false;
    value = std::move(data_queue.front());
    data_queue.pop();
    return true;
  }
};

class thread_pool
{
  std::atomic_bool done;
  thread_safe_queue<std::function<void()> >work_queue;
  std::vector<std::thread> threads;
  join_threads joiner;

  void work_thread()
  {
    while(!done)
    {
      std::function<void()> task;
      if (work_queue.try_pop(task))
      {
        task();
      }
      else
      {
        std::this_thread::yield();
      }
    }
  }
public:
  thread_pool():done(false), joiner(threads)
  {
     unsigned const thread_count = std::thread::hardware_concurrency();
     try
     {
       for (unsigned i = 0; i<thread_count; ++i)
         threads.push_back(std::thread(&thread_pool::work_thread, this));
     }
     catch (...)
     {
       done = true;
       throw;
     }
  }

  ~thread_pool() { done = true; }

  template<typename FunctionType>
  void submit(FunctionType f)
  {
    work_queue.push(std::function<void()>(f));
  }
};



class Registrar : public resip::ThreadIf
{
    public:
      Registrar(resip::SipStack& stack,
                 resip::Data realm,
                 RegDB* mdatabase,
                 const std::vector<resip::Data>& configDomains);
      ~Registrar();

      void thread();
   private:
      resip::SipStack& mStack;
      resip::Data mNameAddr;
      RegDB* mBase;
      std::vector<resip::Data> mConfigDomains;
      thread_pool mThreads;
      std::mutex mut;

      //void analisysRequest(resip::SipMessage* sip);
      void analisysRequest(resip::SipMessage sip);
      void removeAllContacts(resip::SipMessage* sip);
      bool testAuthorization(resip::SipMessage* sip);


      int findDomain(resip::Data& host);
      int addDomain(resip::Data& host);
      int findUser(resip::Data& usr);
      int addUser(resip::Data& usr);
      int findUserDomain(int usr, int dom);
      int addUserDomain(int usr, int dom);
      int findProtocol(resip::Data& protocol);
      int addProtocol(resip::Data& protocol);
      int findOrAddForward(resip::NameAddr& addr, unsigned int reg);
      int findForward(const unsigned int& idp,
                      const unsigned int& idd,
                      const unsigned int& port);
      int addForward(const unsigned int& idp,
                      const unsigned int& idd,
                      const unsigned int& port);
      int findOrAddRegistrar(resip::SipMessage* sip);
      int findRegistrar(const unsigned int& to,
                      const unsigned int& from,
                      resip::Data& callid);
      int addRegistrar(const unsigned int& to,
                        const unsigned int& from,
                        resip::Data& callid);
      void send200(resip::SipMessage* sip, resip::NameAddr add);
      void send400(resip::SipMessage* sip);
      void send401(resip::SipMessage* sip);
      void send403(resip::SipMessage* sip, resip::Data mes);
      void send405(resip::SipMessage* sip, resip::Data meth);
      void send500(resip::SipMessage* sip);

      void loadData();
      void clearData();
      void reloadDomain();

      RegDB::UserRecordList userList;
      RegDB::DomainRecordList domainList;
      RegDB::UserDomainRecordList userDomeinList;
      RegDB::ForwardRecordList forwardList;
      RegDB::ProtocolRecordList protocolList;
      RegDB::AuthorizationRecordList authList;
      RegDB::RegistrarRecordList regList;
      RegDB::RouteRecordList routeList;
      //std::vector<std::future<void>> results(4);
};
}
#endif
