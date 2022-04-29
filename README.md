# ConsistentHashing
Simple implementation of Consistent Hash algo in C++.

Note, hash ring can only be consistent if it's ring size is constant... hence better to initilize ConsistentHashRing class with nodes and then not add dynamically... 
removing is fine here as we don't resize ring.

For more complex production implementation, look at 'ketama hashing': https://www.metabrew.com/article/libketama-consistent-hashing-algo-memcached-clients
and, implementation in envoy proxy: https://github.com/envoyproxy/envoy/blob/main/source/common/upstream/ring_hash_lb.cc
                                    https://www.envoyproxy.io/docs/envoy/latest/intro/arch_overview/upstream/load_balancing/load_balancers
