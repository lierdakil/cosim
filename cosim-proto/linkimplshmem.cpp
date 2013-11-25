#include "linkimplshmem.h"
#include <google/protobuf/io/coded_stream.h>
#include <iostream>

std::shared_ptr<LinkImplSHMEM::mq_t> LinkImplSHMEM::createshm(std::string &name, std::string suffix)
{
    name = "pmns-cosim-"+this->makename()+suffix;
    std::shared_ptr<mq_t> mq;
    for(;;) {
        try{
            mq=std::make_shared<mq_t>(boost::interprocess::create_only, name.data(), 10, sizeof(this->data));
            break;
        } catch (boost::interprocess::interprocess_exception e) {
            if(boost::interprocess::already_exists_error == e.get_error_code()) {
                name = "pmns-cosim-"+this->makename()+suffix;
                continue;
            }
            mq_t::remove(name.data());
            throw e;
        }
    }
    return mq;
}

void LinkImplSHMEM::sendMsg(const google::protobuf::Message &message)
{
    auto sz = this->serialize(message);
#ifndef NDEBUG1
    std::cerr<<">";
    message.PrintDebugString();
#endif
    mq_send->send(this->data,sz,0);
}

void LinkImplSHMEM::recvMsg(google::protobuf::Message &message, std::atomic_bool *abortflag)
{
    mq_t::size_type sz;
    unsigned int prio;
    while(!mq_recv->timed_receive(this->data,sizeof(this->data),sz,prio,
                                  boost::posix_time::microsec_clock::universal_time()+boost::posix_time::seconds(1))
          &&(!abortflag||!*abortflag));
    if(abortflag&&*abortflag)
        throw std::runtime_error("termination requested by user");
    if(sz<sizeof(this->data)) {
        ::google::protobuf::io::CodedInputStream stream(reinterpret_cast< ::google::protobuf::uint8 * >(this->data),sz);
        message.ParsePartialFromCodedStream(&stream);
    }
    else {
        message.Clear();
        auto num_msg = mq_recv->get_num_msg();
        for(;num_msg>0;--num_msg) {
            ::google::protobuf::io::CodedInputStream stream(reinterpret_cast< ::google::protobuf::uint8 * >(this->data),sz);
            message.MergePartialFromCodedStream(&stream);
            if(sz<sizeof(this->data))
                break;
            while(!mq_recv->timed_receive(this->data,sizeof(this->data),sz,prio,
                                          boost::posix_time::microsec_clock::universal_time()+boost::posix_time::seconds(1))
                  &&(!abortflag||!*abortflag));
            if(abortflag&&*abortflag)
                throw std::runtime_error("termination requested by user");
        }
        ///@todo: if sizeof(message)==sizeof(data)/int, there could be additional read?
    }
#ifndef NDEBUG1
    std::cerr<<"<";
    message.PrintDebugString();
#endif
    if(!message.IsInitialized())
        throw std::runtime_error(message.DebugString());
}

void LinkImplSHMEM::setMessageParams(NewConnectionReply &reply)
{
    if(!reply.has_shmemconnparams())
        reply.set_allocated_shmemconnparams(new SHMEMConnParams);
    reply.mutable_shmemconnparams()->set_nameguest(name_guest);
    reply.mutable_shmemconnparams()->set_namehost(name_host);
}

LinkImplSHMEM::LinkImplSHMEM() :
    LinkImpl(),
    managed(true)
{
    mq_send=createshm(name_host,"-host");
    mq_recv=createshm(name_guest,"-guest");
}

LinkImplSHMEM::LinkImplSHMEM(const SHMEMConnParams &params):
    LinkImpl(),
    managed(false),
    name_host(params.namehost()),
    name_guest(params.nameguest())
{
    mq_recv=std::make_shared<mq_t>(boost::interprocess::open_only, name_host.data());
    mq_send=std::make_shared<mq_t>(boost::interprocess::open_only, name_guest.data());
}

LinkImplSHMEM::~LinkImplSHMEM()
{
    if(managed) {
        mq_t::remove(name_host.data());
        mq_t::remove(name_guest.data());
        std::cerr<<"Shmem closed"<<std::endl;
    }
}
