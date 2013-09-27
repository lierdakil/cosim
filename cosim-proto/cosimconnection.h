#ifndef COSIMCONNECTION_H
#define COSIMCONNECTION_H

#include <string>
#include "link.h"
#include <cassert>

class COSIMPROTOSHARED_EXPORT CosimConnection
{
//private:
public:
    std::unique_ptr<Link> link;
    std::string iname;
    double targetTime=NAN;

    struct param_t {
        int length;
        std::string name;
        void* data;
        ParamImports_ParamType type;

        bool operator==(const ParamImports &pi) {
            return length==pi.length() && name==pi.name() && type==pi.type();
        }

#ifndef NDEBUG
        std::string instname;
        ParamImports* infoMessage() const {
            auto info=new ParamImports;
            info->set_name(instname+"::"+name);
            info->set_length(length);
            info->set_type(type);
            return info;
        }
#endif

    #define typecase(type_) \
        case ParamImports_ParamType_pt_ ## type_:\
            for(int i=0; i<length; ++i) \
                reinterpret_cast<type_ *>(data)[i] = (iter++)->type_ ## value();\
            break

        template<class IIter_>
        void setValue(IIter_ &iter) {
#ifndef NDEBUG
            assert(std::shared_ptr<ParamImports>(infoMessage())
                   ->SerializeAsString()==iter->info().SerializeAsString()
                   || !iter->has_info());
#endif
            switch(type) {
            typecase(double);
            typecase(int);
            typecase(float);
            }
        }
    #undef typecase

#ifndef NDEBUG
    #define typecase(type_) \
        case ParamImports_ParamType_pt_ ## type_:\
            for(int i=0; i<length; ++i) \
                { \
                    auto v=m.add_values(); \
                    v->set_ ## type_ ## value(reinterpret_cast<type_ *>(data)[i]);\
                    v->set_allocated_info(infoMessage());\
                } \
            break
#else
    #define typecase(type_) \
        case ParamImports_ParamType_pt_ ## type_:\
            for(int i=0; i<length; ++i) \
                { \
                    auto v=m.add_values(); \
                    v->set_ ## type_ ## value(reinterpret_cast<type_ *>(data)[i]);\
                } \
            break
#endif

        ParameterValues toMessage() const {
            ParameterValues m;
            switch(type) {
            typecase(double);
            typecase(int);
            typecase(float);
            }
            return m;
        }
    #undef typecase

    #define constructor(type_) \
        param_t(std::string name, type_ *data, int length=1) \
            : length(length), name(name), data(data), type(ParamImports_ParamType_pt_ ## type_) {}

        constructor(double)
        constructor(int)
        constructor(float)

    #undef constructor
    };

    struct import_t {
        std::string name;
        std::vector<param_t> params;
    };

    std::vector<import_t> imports;

    std::vector<param_t> exports;

    double simulationTime;

    ExportParametersRequest paramreq;

public:
    CosimConnection();
    void open(std::string host);
    void setInstanceName(const std::string &name);
    void setTargetTime(const double time);
    void addImport(import_t import);
    void init();
    void init(const std::string &name, std::vector<import_t> import, const double time=NAN);
    void registerExport(param_t exprt);
    void registerExports(std::vector<param_t> exprt);
    void checkExports();
    void sendExports();
    void recvImports();

    void sync();

    double step(double estimate);
    double getSimulationTime() const;
};

#endif // COSIMCONNECTION_H
