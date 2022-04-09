#include "hash_test.h"
#include "ConsistentHashRing.h"
#include "endpoint_hash.h"
#include <algorithm>
#include <array>
#include <cstring>
#include <functional>
#include <random>
#include <string>
using namespace std;

TEST(HashTest, CreateRing)
{
    vector<shared_ptr<EndPoint>> endPt_arr = {
        make_shared<EndPoint>("TestSvc1", "192.163.1.10", ip_addr_type::IPV4),
        make_shared<EndPoint>("TestSvc2", "192.163.1.57", ip_addr_type::IPV4),
        make_shared<EndPoint>("TestSvc3", "10.163.68.160", ip_addr_type::IPV4),
        make_shared<EndPoint>("TestSvc4", "10.163.68.161", ip_addr_type::IPV4),
    };

    // Nodes - endpoints, keys - let's say session IDs
    ConsistentHashRing<EndPoint, uint> Ring(endPt_arr);
    ASSERT_EQ(Ring.Size(), 4);
    ASSERT_EQ(Ring.GetNodeIndex(endPt_arr[2]), 2);
    ASSERT_EQ(Ring.NodePredecessor(2)->SvcName, "TestSvc2");
    ASSERT_EQ(Ring.NodeSuccessor(2)->SvcName, "TestSvc4");
}

TEST(HashTest, AddEndpoint)
{
    vector<shared_ptr<EndPoint>> endPt_arr = {
        make_shared<EndPoint>("TestSvc1", "192.163.1.10", ip_addr_type::IPV4),
        make_shared<EndPoint>("TestSvc2", "192.163.1.57", ip_addr_type::IPV4),
        make_shared<EndPoint>("TestSvc3", "10.163.68.160", ip_addr_type::IPV4),
        make_shared<EndPoint>("TestSvc4", "10.163.68.161", ip_addr_type::IPV4),
    };

    // Nodes - endpoints, keys - let's say session IDs
    ConsistentHashRing<EndPoint, uint> Ring(endPt_arr);
    ASSERT_EQ(Ring.Size(), 4);

    std::unordered_map<uint, std::pair<std::string, std::string>> tMap_before_add = {
        {0, {"TestSvc4", "TestSvc2"}}, {1, {"TestSvc1", "TestSvc3"}}, {2, {"TestSvc2", "TestSvc4"}}, {3, {"TestSvc3", "TestSvc1"}}}; // Note the way every service occurs only twice, a simple load balancing.

    for (auto mapIt : tMap_before_add)
    {
        ASSERT_EQ(Ring.NodePredecessor(mapIt.first)->SvcName, mapIt.second.first);
        ASSERT_EQ(Ring.NodeSuccessor(mapIt.first)->SvcName, mapIt.second.second);
    }

    shared_ptr<EndPoint> newPt = make_shared<EndPoint>("TestSvc5", "10.163.68.163", ip_addr_type::IPV4);
    Ring.EmplaceNode(newPt);
    ASSERT_EQ(Ring.Size(), 5);
    ASSERT_EQ(Ring.GetNodeIndex(endPt_arr[2]), 2);
    ASSERT_EQ(Ring.GetNodeIndex(newPt), 4);

    std::unordered_map<uint, std::pair<std::string, std::string>> tMap_after_add = {
        {0, {"TestSvc5", "TestSvc2"}}, {1, {"TestSvc1", "TestSvc3"}}, {2, {"TestSvc2", "TestSvc4"}}, {3, {"TestSvc3", "TestSvc5"}}, {4, {"TestSvc4", "TestSvc1"}}}; // Only nodes 0 and 3 impacted by new node

    for (auto mapIt : tMap_after_add)
    {
        ASSERT_EQ(Ring.NodePredecessor(mapIt.first)->SvcName, mapIt.second.first);
        ASSERT_EQ(Ring.NodeSuccessor(mapIt.first)->SvcName, mapIt.second.second);
    }
}

TEST(HashTest, RemoveEndpoint)
{
    vector<shared_ptr<EndPoint>> endPt_arr = {
        make_shared<EndPoint>("TestSvc1", "192.163.1.10", ip_addr_type::IPV4),
        make_shared<EndPoint>("TestSvc2", "192.163.1.57", ip_addr_type::IPV4),
        make_shared<EndPoint>("TestSvc3", "10.163.68.160", ip_addr_type::IPV4),
        make_shared<EndPoint>("TestSvc4", "10.163.68.161", ip_addr_type::IPV4),
    };

    // Nodes - endpoints, keys - let's say session IDs
    ConsistentHashRing<EndPoint, uint> Ring(endPt_arr);
    ASSERT_EQ(Ring.Size(), 4);

    Ring.RemoveNode(endPt_arr[2]);
    ASSERT_EQ(Ring.Size(), 4);
    ASSERT_EQ(Ring.GetNodeAtIndex(2), nullptr);

    std::unordered_map<uint, std::pair<std::string, std::string>> tMap_before_trim = {
        {0, {"TestSvc4", "TestSvc2"}}, {1, {"TestSvc1", "TestSvc4"}}, {2, {"TestSvc2", "TestSvc4"}}, {3, {"TestSvc2", "TestSvc1"}}}; // changes are minimal

    for (auto mapIt : tMap_before_trim)
    {
        ASSERT_EQ(Ring.NodePredecessor(mapIt.first)->SvcName, mapIt.second.first);
        ASSERT_EQ(Ring.NodeSuccessor(mapIt.first)->SvcName, mapIt.second.second);
    }

    Ring.TrimRing();
    ASSERT_EQ(Ring.Size(), 3);

    std::unordered_map<uint, std::pair<std::string, std::string>> tMap_after_trim = {
        {0, {"TestSvc4", "TestSvc2"}}, {1, {"TestSvc1", "TestSvc4"}}, {2, {"TestSvc2", "TestSvc1"}}, {3, {"TestSvc4", "TestSvc2"}}}; // note how there are more changes after a vector resize... hence we avoid.

    for (auto mapIt : tMap_after_trim)
    {
        ASSERT_EQ(Ring.NodePredecessor(mapIt.first)->SvcName, mapIt.second.first);
        ASSERT_EQ(Ring.NodeSuccessor(mapIt.first)->SvcName, mapIt.second.second);
    }
}

// https://stackoverflow.com/questions/440133/how-do-i-create-a-random-alpha-numeric-string-in-c
template <typename T = std::mt19937>
auto random_generator() -> T
{
    auto constexpr seed_bytes = sizeof(typename T::result_type) * T::state_size;
    auto constexpr seed_len = seed_bytes / sizeof(std::seed_seq::result_type);
    auto seed = std::array<std::seed_seq::result_type, seed_len>();
    std::random_device dev;
    std::generate_n(begin(seed), seed_len, std::ref(dev));
    std::seed_seq seq{};
    seq.generate(seed.begin(), seed.end());
    return T{seq};
}

auto generate_random_alphanumeric_string(std::size_t len) -> std::string
{
    static constexpr auto chars =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    thread_local auto rng = random_generator<>();
    auto dist = std::uniform_int_distribution<>{{}, std::strlen(chars) - 1};
    auto result = std::string(len, '\0');
    std::generate_n(begin(result), len, [&]()
                    { return chars[dist(rng)]; });
    return result;
}

TEST(HashTest, StringHash)
{
    std::random_device rd;                        // obtain a random number from hardware
    std::mt19937 gen(rd());                       // seed the generator
    std::uniform_int_distribution<> distr(5, 20); // random number between 5 ans 20

    std::hash<std::string> str_hasher;
    std::map<uint32_t, int> predMap, successMap;
    int num_nodes = 20;
    for (int i = 0; i < num_nodes; i++)
    {
        std::string randStr = generate_random_alphanumeric_string(distr(gen));
        size_t strHash = str_hasher(randStr);
        uint32_t predecessor = (strHash + (num_nodes - 1)) % num_nodes;
        uint32_t successor = (strHash + 1) % num_nodes;
        predMap[predecessor]++;
        successMap[successor]++;
        std::cout << "randStr: " << randStr << " hash: " << str_hasher(randStr)
                  << " pred: " << predecessor << " successor: " << successor << "\n";
    }

    std::cout << "\n\n **********Mapping predecessor distribution********** \n";
    for (int i = 0; i < num_nodes; i++)
    {
        int predCount = 0;
        try
        {
            predCount = predMap.at(i);
        }
        catch (...)
        {
        }
        std::cout << "0: ";
        for (int j = 0; j < predCount; j++)
            std::cout << "+";
        std::cout << "\n";
    }
    std::cout << "\n   *************************************************** \n";

    std::cout << "\n\n **********Mapping successor distribution********** \n";
    for (int i = 0; i < num_nodes; i++)
    {
        int sCount = 0;
        try
        {
            sCount = successMap.at(i);
        }
        catch (...)
        {
        }
        std::cout << "0: ";
        for (int j = 0; j < sCount; j++)
            std::cout << "+";
        std::cout << "\n";
    }

    std::cout << "\n   **************************************************** \n";
}