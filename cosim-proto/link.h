#ifndef LINK_H
#define LINK_H

#include <memory>
#include <stdexcept>
#include "initial.pb.h"
#include "generalmessage.pb.h"
#include <type_traits>
#include <list>
#include <iostream>
#include "linkimpl.h"
#include <mutex>
#include "init.pb.h"
#include "exportrequest.pb.h"
#include "cosim-proto_global.h"

#define COSIM_PROTOVER 1

class COSIMPROTOSHARED_EXPORT Link : public std::enable_shared_from_this<Link>
{
private:
    std::unique_ptr<LinkImpl> linkImpl;
    void checkError(const GeneralMessage &msg);
    std::atomic_bool *abortflag;
protected:
    int protoVer;
    LinkType linkType;
    void recvGeneralMessage(StepValue &msg);
    void recvGeneralMessage(ParameterValues &msg);
    void sendGeneralMessage(const StepValue &msg);
    void sendGeneralMessage(const ParameterValues &msg);

    void sendMsg(const ::google::protobuf::Message &message);
    void recvMsg(::google::protobuf::Message &message);

    void openLinkImpl(LinkImpl* impl);
    void setMessageParams(NewConnectionReply &reply);
public:
    explicit Link(int protocolVersion, LinkType linkType);

    void sendErrorMessage(Termination_Type type, std::string message);

    void close(Termination_Type type=Termination_Type_TERM_EXIT, std::string message=std::string());
    virtual ~Link() {}

    void sendInitMsg(const Init &msg);
    void recvInitMsg(Init &msg);
    void sendExportParamReq(const ExportParametersRequest& request);
    void recvExportParamReq(ExportParametersRequest& request);
    void sendParameterValues(const ParameterValues &values);
    void recvParameterValues(ParameterValues &values);
    void sendStepValue(double step);
    double recvStepValue();

    void open(NewConnectionReply &reply, bool create);
    std::atomic_bool *getAbortflag() const;
    void setAbortflag(std::atomic_bool *value);
};

#endif // LINK_H
