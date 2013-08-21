#include <iostream>
#include <cosimconnection.h>

using namespace std;
#include <thread>

int main(int, char *argv[])
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    try
    {
        CosimConnection conn;
        conn.open("localhost");

        //initialization
        double imports[3];
        conn.init(argv[1],//this instance name
        {
            {argv[2], //instance to export from
                { // parameters {name,pointer},...
                    {"x",&imports[0]},
                    {"z",&imports[1]}
                }
            },
            {argv[3],
                {
                    {"y",&imports[2]}
                }
            }
        });
        double x,y;
//        conn.registerExport({"x",&x}); //one at a time
//        conn.registerExport({"y",&y});
//        conn.checkExports(); //then check them
        conn.registerExports({
                                 {"x",&x},
                                 {"y",&y}
                             }); //or all at once

        //init end

        double simtime=conn.getSimulationTime();

        //initial values
        x=std::atoi(argv[4]);
        y=1/x;

        for(;;) {
            conn.sync(); //incl. initial values
            auto step=conn.step(1); //advance time step=conn.step(step this instance wants)
            //do calculation
//            assert(step==std::min({x,imports[0],imports[1]}));
//            x=std::rand(); //do calculation
//            std::cout<<imports[0]<<" "<<imports[1]<<std::endl;
//            std::cout<<x<<std::endl;
            //advance solution in time
            simtime+=step;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    google::protobuf::ShutdownProtobufLibrary();
}

