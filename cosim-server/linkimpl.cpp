#include "linkimpl.h"
#include <boost/random/random_device.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <stdexcept>

LinkImpl::LinkImpl()
{
}

std::string LinkImpl::makename()
{
    //generate random name
    std::string name;
    const std::string chars(
                "abcdefghijklmnopqrstuvwxyz"
                "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                );
    boost::random::random_device rng;
    boost::random::uniform_int_distribution<> index_dist(0, chars.size() - 1);
    for(int i = 0; i < 8; ++i) {
        name.push_back(chars[index_dist(rng)]);
    }
    return std::move(name);
}

int LinkImpl::serialize(const google::protobuf::Message &message)
{
    auto sz = message.ByteSize();
    if(!message.SerializeToArray(data,sz))
        throw std::runtime_error("Message failed to serialize (probably too big)");
    return sz;
}
