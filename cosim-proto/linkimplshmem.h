#ifndef LINKIMPLSHMEM_H
#define LINKIMPLSHMEM_H

#include "linkimpl.h"
#include "initial.pb.h"
#include <boost/interprocess/ipc/message_queue.hpp>

class LinkImplSHMEM : public LinkImpl
{
private:
    const bool managed;
    using mq_t=boost::interprocess::message_queue;
    std::string name_host, name_guest;
    std::shared_ptr<mq_t> mq_send, mq_recv;
    std::shared_ptr<mq_t> createshm(std::string &name,std::string suffix);
public:
    LinkImplSHMEM();
    LinkImplSHMEM(const SHMEMConnParams& params);
    ~LinkImplSHMEM();
    void sendMsg(const google::protobuf::Message &message) override;
    void recvMsg(google::protobuf::Message &message, std::atomic_bool *abortflag) override;

    void setMessageParams(NewConnectionReply &reply);
};

#endif // LINKIMPLSHMEM_H
