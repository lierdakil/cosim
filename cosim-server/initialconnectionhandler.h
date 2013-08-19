#ifndef INITIALCONNECTIONHANDLER_H
#define INITIALCONNECTIONHANDLER_H

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/read.hpp>
#include <condition_variable>
#include <memory>

#include <link.h>
#include <messagesize.pb.h>

class InitialConnectionHandler : public std::enable_shared_from_this<InitialConnectionHandler>
{
private:
    char data[81920];

public:
    InitialConnectionHandler();
    std::shared_ptr<Link> handle(boost::asio::ip::tcp::socket& socket);
};

#endif // INITIALCONNECTIONHANDLER_H
