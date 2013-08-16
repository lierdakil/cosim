#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include "initialconnectionhandler.h"
#include "guestlinkhandler.h"

#include <signal.h>
#include <thread>

void sighandler(int) {
    std::list<std::thread> closeThreads;
    {
        std::lock_guard<std::mutex> lock(GuestLinkHandler::guestListMutex);
        for(auto i : GuestLinkHandler::guestList)
            closeThreads.push_back(std::thread([i] {i->close();}));
    }
    for(auto &i : closeThreads)
        i.join();
    signal(SIGINT, SIG_DFL);
    kill(getpid(), SIGINT);
}

int main()
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;

//    signal(SIGTERM, sighandler);
//    signal(SIGABRT, sighandler);
    signal(SIGINT, sighandler);
    try
    {
        boost::asio::io_service io_service;

        using boost::asio::ip::tcp;

        tcp::acceptor acceptor(io_service,
                               tcp::endpoint(tcp::v4(), 1346));

        std::thread([] {
            std::cout<<"Press enter when all guests are started"<<std::endl;
            std::cin.get();
            GuestLinkHandler::signalGuestsConnected();
            std::cout<<"OK"<<std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(5));
//            sighandler(SIGINT);
        }).detach();

        for (;;)
        {
            boost::system::error_code ec;
            tcp::socket socket(io_service);
            acceptor.accept(socket,ec);
            if(!ec) {
                auto link = std::make_shared<InitialConnectionHandler>()->handle(socket);

                if(link)
                    std::make_shared<GuestLinkHandler>()->handle(link);
            }
        }

    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    google::protobuf::ShutdownProtobufLibrary();

    return 0;
}

