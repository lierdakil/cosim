#include "guestlinkhandler.h"
#include <future>

bool GuestLinkHandler::allGuestsConnected;
std::condition_variable GuestLinkHandler::cvAllGuestsConnected;
std::mutex GuestLinkHandler::cvAllGuestsConnectedMutex;
std::atomic<double> GuestLinkHandler::simtime;
std::atomic<double> GuestLinkHandler::stepEstimate;

std::list<std::shared_ptr<GuestLinkHandler> > GuestLinkHandler::guestList;
std::mutex GuestLinkHandler::guestListMutex;


bool GuestLinkHandler::barrierf(bool wakeup) {
    static std::unique_ptr<barrier> barrier_;
    static std::mutex barrierMutex;
    {
        std::lock_guard<std::mutex> lock(barrierMutex);
        if(!barrier_){
            barrier_.reset(new barrier(guestList.size()));
        }
    }
    if(wakeup) {
        barrier_->wakeup();
        return false;
    }
    return barrier_->wait(&abortflag);
}

void GuestLinkHandler::waitGuestsConnected()
{
    std::unique_lock<std::mutex> lock(cvAllGuestsConnectedMutex);
    cvAllGuestsConnected.wait(lock,[]{return allGuestsConnected;});
}

void GuestLinkHandler::addSubscriber(const std::shared_ptr<GuestLinkHandler> &sub,ExportParametersRequest &epr)
{
    std::vector<int> vec;
    int shift;
    auto &tmp = sub->getImports(instanceName(),&shift);
    if(tmp.size()>0) {
        addExports(tmp,epr);
        for(auto &elem : tmp) {
            int j=0;
            for(auto &exprt : epr.exports()) {
                if(exprt.name()==elem.name()) {
                    vec.push_back(j);
                    break;
                }
                j+=exprt.length();
            }
        }
        mySubscribers.push_back({sub,shift,std::move(vec)});
    }
}

void GuestLinkHandler::removeSubscriber(const std::shared_ptr<GuestLinkHandler> &sub)
{
    auto i = std::find(mySubscribers.begin(), mySubscribers.end(), sub);
    if(i!=mySubscribers.end())
        mySubscribers.erase(i);
}

const::google::protobuf::RepeatedPtrField<ParamImports> &GuestLinkHandler::getImports(const std::string &instance, int *shift) const
{
    *shift=0;
    for(auto &i: initMessage.imports()) {
        if(i.name()==instance)
            return i.params();
        for(auto &j : i.params())
            *shift += j.length();

    }
    static auto empty=::google::protobuf::RepeatedPtrField<ParamImports>();
    return empty;
}

void GuestLinkHandler::addExports(const::google::protobuf::RepeatedPtrField<ParamImports> &list,ExportParametersRequest &epr)
{
    if(eprLock)
        throw std::runtime_error("Already sent ExportParametersRequest");
    for(const ParamImports &i : list) {
        bool hasi=false;
        for(const ParamImports &j : epr.exports()) {
            hasi = i.name()==j.name();
            if(hasi)
                break;
        }
        if(!hasi) {
            epr.mutable_exports()->Add()->CopyFrom(i);
        }
    }
}

void GuestLinkHandler::propagateExportsAndSubscribe(ExportParametersRequest &epr)
{
    std::lock_guard<decltype(guestListMutex)> lock(guestListMutex);
    for(const std::shared_ptr<GuestLinkHandler> &i : guestList) {
        addSubscriber(i,epr);
    }
}

const std::string &GuestLinkHandler::instanceName() const
{
    return initMessage.name();
}

void GuestLinkHandler::setSubscriberValues()
{
    for(auto &s : mySubscribers) {
        for(unsigned int i=0; i<s.mapping.size(); ++i)
        {
            s.handler->valuesImported.mutable_values(i+s.shift)->CopyFrom(valuesExported.values(s.mapping[i]));
        }
    }
}

GuestLinkHandler::GuestLinkHandler() :
    std::enable_shared_from_this<GuestLinkHandler>()
{
}

void GuestLinkHandler::handle(std::shared_ptr<Link> link)
{
    auto self = shared_from_this();
    guestList.push_back(self);
    myThread=std::make_shared<std::thread> ( [this,self,link] {
        try {
            std::cerr<<"Link parameters sent"<<std::endl;

            link->recvInitMsg(initMessage);
            valuesImported.Clear();
            for(auto &i : initMessage.imports()) {
                for(auto &p : i.params()) {
                    for(int c=0; c<p.length(); ++c)
                        valuesImported.add_values();
                }
            }
            std::cerr<<"Init msg received"<<std::endl;
            std::cerr<<"Instance "<<instanceName()<<std::endl;
            waitGuestsConnected();

            std::cerr<<"All guests connected"<<std::endl;

            //Main init phase
            {
                ExportParametersRequest epr;

                propagateExportsAndSubscribe(epr);

                epr.set_simulationtime(simtime);
                link->sendExportParamReq(epr);
            }

            //init finished

            link->setAbortflag(&abortflag);
            //main loop
            double mysimtime=simtime;
            for(;;) {
                if(barrierf())//exactly one thread returns true
                    stepEstimate.store(NAN);

                std::cerr<<"Loop start t="<<mysimtime<<std::endl;

                link->recvParameterValues(valuesExported);

                {
                    std::lock_guard<decltype(guestListMutex)> lock(guestListMutex);
                    setSubscriberValues();
                }
                std::cerr<<"Values received"<<std::endl;
                //barrier sync
                barrierf();

                link->sendParameterValues(valuesImported);
                std::cerr<<"Values sent"<<std::endl;

                double se = link->recvStepValue();
                double sc=NAN;
                while(!stepEstimate.compare_exchange_weak(sc,se) && (sc>se));
                //wait for other estimates
                std::cerr<<"Step decided"<<std::endl;
                //barrier sync
                barrierf();

                link->sendStepValue(stepEstimate);
                std::cerr<<"Step sent"<<std::endl;

                mysimtime+=stepEstimate;
                //exit condition
            }
        } catch (Termination &e) {
            std::cerr<<"Guest "<<instanceName()<<" terminated: "<<
                    e.message()<<std::endl;
        } catch (std::exception &e) {
            link->sendErrorMessage(Termination_Type_TERM_ERROR,e.what());
        }
        std::lock_guard<decltype(guestListMutex)> lock(guestListMutex);
        for(const auto &i : guestList) {
            i->removeSubscriber(self);
        }
        guestList.remove(self);
        std::cout<<"thread finish"<<std::endl;
        if(!abortflag)
            myThread->detach();
    });

    //main loop finished
}

void GuestLinkHandler::close()
{
    if(myThread->joinable()) {
        ///@todo send signal to abort operation
        bool flag=false;
        abortflag.compare_exchange_strong(flag,true);
        barrierf(true);
        std::cout<<"thread join"<<std::endl;
        myThread->join();
        std::cout<<"thread unjoin"<<std::endl;
    }
}

void GuestLinkHandler::signalGuestsConnected()
{
    std::lock_guard<std::mutex> lock(cvAllGuestsConnectedMutex);
    allGuestsConnected=true;
    cvAllGuestsConnected.notify_all();
}
