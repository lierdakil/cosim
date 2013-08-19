#include "link.h"
#include <boost/random/random_device.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include "linkimplshmem.h"

void Link::checkError(const GeneralMessage &msg)
{
    if(msg.has_msg()) {
        throw std::runtime_error(msg.msg().message());
    }
}
std::atomic_bool *Link::getAbortflag() const
{
    return abortflag;
}

void Link::setAbortflag(std::atomic_bool *value)
{
    abortflag = value;
}


void Link::recvGeneralMessage(StepValue &msg)
{
    GeneralMessage m;
    m.set_allocated_step(&msg);
    try {
        recvMsg(m);
    } catch(...) {
        m.release_values();
        throw;
    }
    m.release_step();
    checkError(m);
}

void Link::recvGeneralMessage(ParameterValues &msg)
{
    GeneralMessage m;
    m.set_allocated_values(&msg);
    try {
        recvMsg(m);
    } catch(...) {
        m.release_values();
        throw;
    }

    m.release_values();
    checkError(m);
}

void Link::sendGeneralMessage(const StepValue &msg)
{
    GeneralMessage m;
    m.set_allocated_step(const_cast<StepValue*>(&msg));
    try {
        sendMsg(m);
    } catch (...) {
        m.release_step();
        throw;
    }
    m.release_step();
}

void Link::sendGeneralMessage(const ParameterValues &msg)
{
    GeneralMessage m;
    m.set_allocated_values(const_cast<ParameterValues*>(&msg));
    try {
    sendMsg(m);
    } catch(...) {
        m.release_values();
        throw;
    }

    m.release_values();
}

Link::Link(int protocolVersion, LinkType linkType) :
    abortflag(nullptr),
    protoVer(protocolVersion),
    linkType(linkType)
{}

void Link::sendMsg(const google::protobuf::Message &message)
{
    if(linkImpl)
        linkImpl->sendMsg(message);
    else
        throw std::runtime_error("Link is not open yet");
}

void Link::recvMsg(google::protobuf::Message &message)
{
    if(linkImpl)
        linkImpl->recvMsg(message, abortflag);
    else
        throw std::runtime_error("Link is not open yet");
}

void Link::openLinkImpl(LinkImpl *impl)
{
    linkImpl.reset(impl);
}

void Link::setMessageParams(NewConnectionReply &reply)
{
    linkImpl->setMessageParams(reply);
}

void Link::sendErrorMessage(Termination_Type type, std::string message)
{
    GeneralMessage msg;
    msg.set_allocated_msg(new Termination);
    msg.mutable_msg()->set_type(type);
    msg.mutable_msg()->set_message(message);
    sendMsg(msg);
}

void Link::close(Termination_Type type, std::string message)
{
    sendErrorMessage(type,message);
    linkImpl.reset();
}

void Link::sendInitMsg(const Init &msg)
{
    sendMsg(msg);
}

void Link::recvInitMsg(Init &msg)
{
    recvMsg(msg);
}

void Link::sendExportParamReq(const ExportParametersRequest &request)
{
    sendMsg(request);
}

void Link::recvExportParamReq(ExportParametersRequest &request)
{
    recvMsg(request);
}

void Link::sendParameterValues(const ParameterValues &values)
{
    sendGeneralMessage(values);
}

void Link::recvParameterValues(ParameterValues &values)
{
    recvGeneralMessage(values);
}

void Link::sendStepValue(double step)
{
    StepValue s;
    s.set_step(step);
    sendGeneralMessage(s);
}

double Link::recvStepValue()
{
    StepValue s;
    recvGeneralMessage(s);
    return s.step();
}

void Link::open(NewConnectionReply &reply, bool create)
{
    switch(linkType) {
    case SHMEM:
        if(create){
            openLinkImpl(new LinkImplSHMEM);
            setMessageParams(reply);
        }
        else
            openLinkImpl(new LinkImplSHMEM(reply.shmemconnparams()));
        break;
    default:
        throw std::runtime_error("Unimplimented");
    }
}
