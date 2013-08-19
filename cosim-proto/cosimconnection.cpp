#include "cosimconnection.h"

#include <boost/asio.hpp>
#include "messagesize.pb.h"


double CosimConnection::getSimulationTime() const
{
    return simulationTime;
}

CosimConnection::CosimConnection()
{

}

void CosimConnection::open(std::string host)
{
    using boost::asio::ip::tcp;
    boost::asio::io_service io_service;

    tcp::socket s(io_service);
    tcp::resolver resolver(io_service);
    boost::asio::connect(s, resolver.resolve({host, "1346"}));

    //initial connection
    NewConnectionRequest req;
    req.set_protocolversion(COSIM_PROTOVER);
    MessageSize sz;
    sz.set_size(req.ByteSize());
    boost::asio::write(s, boost::asio::buffer(sz.SerializeAsString()));
    boost::asio::write(s, boost::asio::buffer(req.SerializeAsString()));

    std::vector<char> buf;
    buf.resize(sz.ByteSize());
    boost::asio::read(s,boost::asio::buffer(buf));
    sz.ParseFromArray(buf.data(),buf.size());
    buf.resize(sz.size());
    boost::asio::read(s,boost::asio::buffer(buf));
    NewConnectionReply reply;
    reply.ParseFromArray(buf.data(),buf.size());

    //link establish
    link.reset(new Link(reply.protocolversion(),reply.uselinktype()));

    link->open(reply,false);
}

void CosimConnection::setInstanceName(const std::string &name)
{
    iname=name;
}

void CosimConnection::setTargetTime(const double time)
{
    targetTime=time;
}

void CosimConnection::addImport(CosimConnection::import_t import)
{
    imports.push_back(std::move(import));
}


void CosimConnection::init()
{
    Init init;
    init.set_name(iname);
    if(!std::isnan(targetTime))
        init.set_targettime(targetTime);
    for(auto &i : imports) {
        auto import = init.add_imports();
        import->set_name(i.name);
        for(auto &p : i.params) {
#ifndef NDEBUG
            p.instname=i.name;
#endif
            auto param = import->add_params();
            param->set_name(p.name);
            if(p.length!=param->type())//check default
                param->set_type(p.type);
            if(p.length!=param->length())//check default
                param->set_length(p.length);
        }
    }
    link->sendInitMsg(init);
    link->recvExportParamReq(paramreq);
}

void CosimConnection::init(const std::string &name, std::vector<import_t> import, const double time)
{
    setInstanceName(name);
    setTargetTime(time);
    imports=std::move(import);
    init();
}

void CosimConnection::registerExport(CosimConnection::param_t exprt)
{
    exports.push_back(exprt);
}

void CosimConnection::registerExports(std::vector<CosimConnection::param_t> exprt)
{
    exports=std::move(exprt);
    checkExports();
}

void CosimConnection::checkExports()
{
#ifndef NDEBUG
    std::for_each(exports.begin(),exports.end(),[this] (param_t &p) {p.instname=iname;});
#endif
    for(int i=0; i<paramreq.exports_size(); ++i) {
        auto j=exports.end();
        if((j=std::find(exports.begin(), exports.end(), paramreq.exports(i)))==exports.end()) {
            std::string message="Unable to provide parameter "+paramreq.exports(i).name();
            link->sendErrorMessage(Termination_Type_TERM_ERROR,
                                   message);
            throw std::runtime_error(message);
        }
        else{
            std::swap(exports[i],*j);
        }
    }
    exports.resize(paramreq.exports_size(),{"error",(double*)nullptr});
}

void CosimConnection::sendExports()
{
    ParameterValues values;
    std::for_each(exports.begin(), exports.end(), [this,&values] (const param_t &p) {
        values.MergeFrom(p.toMessage());
    });
    link->sendParameterValues(values);
}

void CosimConnection::recvImports()
{
    ParameterValues values;
    link->recvParameterValues(values);
    auto i = values.values().begin();
    std::for_each(imports.begin(), imports.end(), [&i] (import_t &imp) {
        std::for_each(imp.params.begin(),imp.params.end(),[&i,&imp] (param_t &p) {
            p.setValue(i);
        });
    });
}

void CosimConnection::sync()
{
    sendExports();
    recvImports();
}

double CosimConnection::step(double estimate)
{
    link->sendStepValue(estimate);
    return link->recvStepValue();
}
