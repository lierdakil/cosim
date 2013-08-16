#ifndef LINKIMPL_H
#define LINKIMPL_H

#include "../cosim-proto/initial.pb.h"
#include <atomic>

class LinkImpl
{
protected:
    LinkImpl();
    std::string makename();
    char data[81920];
    int serialize(const ::google::protobuf::Message &message);
public:
    virtual void sendMsg(const ::google::protobuf::Message &message)=0;
    virtual void recvMsg(::google::protobuf::Message &message, std::atomic_bool *abortflag)=0;

    virtual void setMessageParams(NewConnectionReply &reply)=0;

    virtual ~LinkImpl() {}
};

#endif // LINKIMPL_H
