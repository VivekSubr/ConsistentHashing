#include <vector>
#include <memory>
#include <functional>
#include <algorithm>
#include <iostream>
#pragma once

template <typename node_t, typename key_t>
class ConsistentHashRing
{
public:
  ConsistentHashRing() = default;

  ConsistentHashRing(const std::vector<node_t> &nodes)
  {
    std::copy_if(nodes.begin(), nodes.end(), std::back_inserter(m_nodes), 
                                      [this](node_t node){return node != this->nullNode();} );
  }

  ConsistentHashRing(const node_t &node)
  {
    if(node != nullNode()) emplaceNode(node);
  }

  ~ConsistentHashRing() = default;
  
  uint32_t size() const { return m_nodes.size(); }
  
  std::vector<node_t> getNodes() const 
  { 
    std::vector<node_t> ret;
    std::copy_if(m_nodes.begin(), m_nodes.end(), std::back_inserter(ret), 
                                      [this](node_t node){return node != this->nullNode();} );
    return ret; 
  }

  uint32_t getNodeIndex(node_t node) const
  {
    for(int i=0; i<m_nodes.size(); i++) 
    { //no better way unless we enforce that data will be sorted
      if(m_nodes[i] != nullNode() && *node == *m_nodes[i]) return i;
    } return UINT32_MAX;
  }

  node_t nodePredecessor(key_t key) const
  { //get the node preceding modulo of hash of this key
    size_t number_nodes = m_nodes.size();
    size_t i = m_key_hasher(key, number_nodes);
    uint32_t index = (i + (number_nodes - 1)) % number_nodes; // i + (n-1), basically go around the ring to the node get behind... the predecessor.
    if(m_nodes[index] != nullNode()) return m_nodes[index]; //need to check for gaps in the ring.
    //else, walk the ring to find next non-null predecessor
    int walked_nodes = 1;
    int nIndex = index;
    while (walked_nodes <= number_nodes)
    {
      nIndex = nIndex - 1;
      if (nIndex < 0) nIndex = number_nodes - 1;
      node_t nNode = m_nodes[nIndex];
      if (nNode != nullNode()) return nNode;
      walked_nodes++;
    }

    return nullNode(); //all null!
  }

  node_t nodeSuccessor(key_t key) const
  { //get node succeeding hash of this key
    size_t number_nodes = m_nodes.size();
    size_t i = m_key_hasher(key, number_nodes);
    uint32_t index = (i + 1) % number_nodes;
    if(m_nodes[index] != nullNode()) return m_nodes[index];
    //else, walk the ring to find next non-null successor
    int walked_nodes = 1;
    int nIndex = index;
    while (walked_nodes <= number_nodes)
    {
      nIndex = nIndex + 1;
      if (nIndex >= number_nodes) nIndex = 0;
      node_t nNode = m_nodes[nIndex];
      if (nNode != nullNode()) return nNode;
      walked_nodes++;
    }

    return nullNode(); //all null!
  }

  node_t getNodeAtIndex(uint32_t index) const
  { 
    try { return m_nodes.at(index); }
    catch(const std::out_of_range& oor) {
      std::cout<<"Failed to get node at index "<<index<<"\n";
      return nullNode();
    }
  }

  void pushNode(const node_t &node)    { m_nodes.push_back(node); }
  void emplaceNode(const node_t &node) { m_nodes.emplace_back(node); }

  bool replaceNode(const node_t &node)
  { 
    for(int i =0; i < m_nodes.size(); i++) {
      if(m_nodes[i] == nullNode()) {
         m_nodes[i] = node; //since we try to keep ring size constant for consistency, remove op leaves gaps
         return true;
      }
    }

    emplaceNode(node); //if no gaps, expand ring, which is less impactful op than resizing.
    return true;
  }

  bool replaceNode(const node_t &old_node, const node_t &new_node)
  {
    uint32_t index = getNodeIndex(old_node);
    if (index == UINT32_MAX) 
    {
      std::cout<<"Failed to replace node\n";
      return false;
    }

    m_nodes[index] = new_node;
    return true;
  }

  bool removeNode(const node_t &node)
  { //resizing the ring would mean hashes become inconsistent
    uint32_t index = getNodeIndex(node);
    if (index == UINT32_MAX) 
    {
      std::cout<<"Failed to remove node\n";
      return false;
    }

    m_nodes[index] = nullNode(); //hence we simply leave gaps
    return true;
  }

  void trimRing() 
  { //get rid of null nodes
    std::vector<node_t> new_nodes_arr;
    for(const auto& it : m_nodes) {
      if(it != nullNode()) new_nodes_arr.emplace_back(it);
    }

    m_nodes = new_nodes_arr;
  }

  bool isRingEmpty() const
  { //is vector empty or else are all nodes null? 
    if(m_nodes.empty()) return true;
    return std::all_of(m_nodes.begin(), m_nodes.end(), [this](node_t node) { return node == this->nullNode(); });
  }

  uint32_t getNumberNodes() const
  { //count non-null nodes
    return std::count_if(m_nodes.begin(), m_nodes.end(), [this](node_t node) { return node != this->nullNode(); });
  }

  void setHasher(const std::function<size_t(key_t, uint32_t)>& key_hasher) { m_key_hasher = key_hasher; }

private:
  std::vector<node_t>     m_nodes; //nodes typically represent servers/endpoints for load balancing
  std::hash<key_t>        m_key_std_hash; //by default, std::hash
  std::function<size_t(key_t, uint32_t)> m_key_hasher = [=](key_t key, uint32_t nNodes)->size_t { return m_key_std_hash(key); };

  //https://stackoverflow.com/questions/41853159/how-to-detect-if-a-type-is-shared-ptr-at-compile-time
  template <class T> struct is_shared_ptr : std::false_type {};
  template <class T> struct is_shared_ptr<std::shared_ptr<T> > : std::true_type {};
  node_t nullNode() const
  { 
    if(is_shared_ptr<node_t>{})   return nullptr;   
    if(std::is_pointer<node_t>{}) return nullptr; //plain pointer
    return node_t(); //if all else fails, try default constructing 
  } 
};