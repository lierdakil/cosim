#ifndef GUESTLINKHANDLER_H
#define GUESTLINKHANDLER_H

#include "link.h"
#include <atomic>
#include <condition_variable>
#include <memory>
#include <thread>

class GuestLinkHandler : public std::enable_shared_from_this<GuestLinkHandler>
{
    friend void sighandler(int);
private://static
    static bool allGuestsConnected;
    static std::condition_variable cvAllGuestsConnected;
    static std::mutex cvAllGuestsConnectedMutex;
    static std::atomic<double> simtime;
    static std::atomic<double> stepEstimate;

    static std::list<std::shared_ptr<GuestLinkHandler> > guestList;
    static std::mutex guestListMutex;

    class barrier
    {
    public:
        barrier(unsigned int count)
            : m_threshold(count), m_count(count), m_generation(0)
        {
        }

        bool wait(std::atomic_bool *abortflag)
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            unsigned int gen = m_generation;

            if (--m_count == 0)
            {
                m_generation++;
                m_count = m_threshold;
                m_cond.notify_all();
                return true;
            }

            m_cond.wait(lock, [gen,this,&abortflag] {return gen!=m_generation || *abortflag;});
            if(*abortflag)
                throw std::runtime_error("termination requested by user");
            return false;
        }

        void wakeup() {
            m_cond.notify_all();
        }

    private:
        std::mutex m_mutex;
        std::condition_variable m_cond;
        unsigned int m_threshold;
        unsigned int m_count;
        unsigned int m_generation;
    };

    bool barrierf(bool wakeup=false);

    static void waitGuestsConnected();
private:
    Init initMessage;
//    ExportParametersRequest epr;
    bool eprLock=false;
    ParameterValues valuesImported;
    ParameterValues valuesExported;
    struct subscriber_t {
        std::shared_ptr<GuestLinkHandler> handler;
        int shift;
        std::vector<int> mapping;
        bool operator==(std::shared_ptr<GuestLinkHandler> handler_) {
            return handler_==handler;
        }
    };
    std::vector<subscriber_t> mySubscribers;


    void addSubscriber(const std::shared_ptr<GuestLinkHandler> &sub, ExportParametersRequest &epr);
    void removeSubscriber(const std::shared_ptr<GuestLinkHandler> &sub);

    const::google::protobuf::RepeatedPtrField<ParamImports> &getImports(const std::string &instance, int *shift) const;
    void addExports(const ::google::protobuf::RepeatedPtrField<ParamImports> &list, ExportParametersRequest &epr);

    void propagateExportsAndSubscribe(ExportParametersRequest &epr);

    const std::string &instanceName() const;

    void setSubscriberValues();

    std::shared_ptr<std::thread> myThread;

    std::atomic_bool abortflag;
public:
    GuestLinkHandler();
    void handle(std::shared_ptr<Link> link);
    void close();

    static void signalGuestsConnected();
};

#endif // GUESTLINKHANDLER_H

