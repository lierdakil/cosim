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
        double a,b,cn;
        conn.init(argv[1],//this instance name
        {
            {argv[2], //instance to export from
                { // parameters {name,pointer},...
                    {"model_1.a",&a,1},
                    {"model_1.b",&b,1},
                    {"model.cnst",&cn,1},
                }
            },
        });
        double c;
//        conn.registerExport({"x",&x}); //one at a time
//        conn.registerExport({"y",&y});
//        conn.checkExports(); //then check them
        conn.registerExports({
                                 {"c",&c,1}
                             }); //or all at once

        //init end

        double simtime=conn.getSimulationTime();

        //initial values
        c=0;

        for(;;) {
            conn.sync(); //incl. initial values
            std::cout<<cn<<std::endl;
            auto step=conn.step(1); //advance time step=conn.step(step this instance wants)
            //do calculation
//            assert(step==std::min({x,imports[0],imports[1]}));
//            x=std::rand(); //do calculation
            c=a+b;
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

