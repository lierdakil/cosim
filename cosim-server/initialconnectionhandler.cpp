#include "initialconnectionhandler.h"
#include <future>

InitialConnectionHandler::InitialConnectionHandler()
{
}

std::shared_ptr<Link> InitialConnectionHandler::handle(boost::asio::ip::tcp::socket &socket)
{
    NewConnectionRequest request;
    NewConnectionReply reply;
    MessageSize sz;
    try
    {
        std::cerr<<"New connection started"<<std::endl;
        sz.set_size(0);
        boost::asio::read(socket,boost::asio::buffer(data,sz.ByteSize()));
        if(!sz.ParseFromArray(data,sz.ByteSize()))
            throw std::runtime_error("Failed to parse "+sz.GetTypeName());
        std::size_t n = boost::asio::read(socket,boost::asio::buffer(data,sz.size()));
        if(!request.ParseFromArray(data,n))
            throw std::runtime_error("Failed to parse "+request.GetTypeName());
        std::cerr<<"New connection request parsed"<<std::endl;
        int pv = std::min((google::protobuf::uint32)COSIM_PROTOVER,request.protocolversion());
        reply.Clear();
        reply.set_protocolversion(pv);
        std::cerr<<"Protocol verson "<<pv<<std::endl;
        ///@todo: check suggestedLinkType
        LinkType lt = SHMEM;
        reply.set_uselinktype(lt);
        auto link=std::make_shared<Link>(pv,lt);

        link->open(reply,true);

        reply.set_result(NewConnectionReply_Status_OK);
        n=reply.ByteSize();
        sz.set_size(n);
        sz.SerializeToArray(data,sz.ByteSize());
        reply.SerializeToArray(&data[sz.ByteSize()],n);
        boost::asio::write(socket, boost::asio::buffer(data, n+sz.ByteSize()));
        socket.close();
        return std::move(link);
    }
    catch (std::exception& e)
    {
        reply.Clear();
        reply.set_result(NewConnectionReply_Status_ERROR);
        reply.set_uselinktype(TCP);
        reply.set_protocolversion(0);
        reply.set_message(e.what());
        std::size_t n =reply.ByteSize();
        sz.set_size(n);
        sz.SerializeToArray(data,sz.ByteSize());
        reply.SerializeToArray(&data[sz.ByteSize()],n);
        boost::asio::write(socket, boost::asio::buffer(data, n+sz.ByteSize()));
        socket.close();
        return std::shared_ptr<Link>();
    }
}
