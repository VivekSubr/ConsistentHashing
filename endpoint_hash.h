#include <string>
#include <vector>
#include <iostream>
#include "boost/functional/hash.hpp"
#pragma once

enum class ip_addr_type { IPV4, IPV6 };
struct EndPoint 
{
    std::string  SvcName;
    std::string  Ip_addr;
    ip_addr_type Ip_type;

    EndPoint(const std::string& name, const std::string& ipAddr, ip_addr_type type):
            SvcName(name), Ip_addr(ipAddr), Ip_type(type) 
    {
    }

    //Copy constructor
    EndPoint(const EndPoint& pt) {
       SvcName = pt.SvcName;
       Ip_addr = pt.Ip_addr;
       Ip_type = pt.Ip_type;
    }

    //Move constructor 
    EndPoint(EndPoint&& pt) {
       SvcName = std::move(pt.SvcName);
       Ip_addr = std::move(pt.Ip_addr);
       Ip_type = pt.Ip_type;
    }

    bool operator<(const EndPoint& r) const
    {    
       return Ip_addr < r.Ip_addr;
    }

    bool operator>(const EndPoint& r) const
    {    
       return Ip_addr > r.Ip_addr;
    }
    
    bool operator==(const EndPoint& r) const
    {    
       return SvcName == r.SvcName && Ip_addr == r.Ip_addr && Ip_type == r.Ip_type;
    }

    size_t Hash(size_t seed = 0) const
    {
        boost::hash_combine(seed, std::hash<std::string>{}(SvcName));
        boost::hash_combine(seed, std::hash<std::string>{}(Ip_addr));
        return seed;
    }
};

struct EndPointHasher
{
  std::size_t operator()(EndPoint const& k) const
  {
    return k.Hash();
  }
};
