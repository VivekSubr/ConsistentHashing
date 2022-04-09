#include <vector>
#include <memory>
#include <functional>
#include <algorithm>
#pragma once

template <typename node_t, typename key_t>
class ConsistentHashRing
{
  using nodePtr = std::shared_ptr<node_t>;

public:
  ConsistentHashRing<node_t, key_t>(const std::vector<nodePtr> &nodes) : m_nodes(nodes)
  {
  }

  virtual ~ConsistentHashRing() = default;

  uint32_t Size() { return m_nodes.size(); }

  uint32_t GetNodeIndex(nodePtr node)
  {
    auto it = std::find(m_nodes.begin(), m_nodes.end(), node);
    if (it != m_nodes.end()) return it - m_nodes.begin();
    return UINT32_MAX;
  }

  nodePtr NodePredecessor(key_t key)
  {
    size_t i = m_key_hasher(key);
    size_t number_nodes = m_nodes.size();
    uint32_t index = (i + (number_nodes - 1)) % number_nodes;
    if(m_nodes[index] != nullptr) return m_nodes[index];

    //else, walk the ring to find next non-null predecessor
    int walked_nodes = 1;
    int nIndex = index;
    while (walked_nodes <= number_nodes)
    {
      nIndex = nIndex - 1;
      if (nIndex < 0) nIndex = number_nodes - 1;
      nodePtr nNode = m_nodes[nIndex];
      if (nNode != nullptr) return nNode;
      walked_nodes++;
    }

    return nullptr; //all null!
  }

  nodePtr NodeSuccessor(key_t key)
  {
    size_t i = m_key_hasher(key);
    size_t number_nodes = m_nodes.size();
    uint32_t index = (i + 1) % number_nodes;
    if(m_nodes[index] != nullptr) return m_nodes[index];

    //else, walk the ring to find next non-null successor
    int walked_nodes = 1;
    int nIndex = index;
    while (walked_nodes <= number_nodes)
    {
      nIndex = nIndex + 1;
      if (nIndex >= number_nodes) nIndex = 0;
      nodePtr nNode = m_nodes[nIndex];
      if (nNode != nullptr) return nNode;
      walked_nodes++;
    }

    return nullptr; //all null!
  }

  nodePtr GetNodeAtIndex(uint32_t index) 
  { 
    try { return m_nodes.at(index); }
    catch(const std::out_of_range& oor) {
      std::cout << "Out of Range error: " << oor.what() << '\n';
      return nullptr;
    }
  }

  void PushNode(const nodePtr &node)    { m_nodes.push_back(node); }
  void EmplaceNode(const nodePtr &node) { m_nodes.emplace_back(node); }

  bool ReplaceNode(const nodePtr &node)
  {
    for(int i =0; i < m_nodes.size(); i++) {
      if(m_nodes[i] == nullptr) {
         m_nodes[i] = node;
         return true;
      }
    }

    EmplaceNode(node);
    return true;
  }

  bool ReplaceNode(const nodePtr &old_node, const nodePtr &new_node)
  {
    uint32_t index = GetNodeIndex(old_node);
    if (index == UINT32_MAX) return false;
    m_nodes[index] = new_node;
    return true;
  }

  bool RemoveNode(const nodePtr &node)
  { //resizing the ring would mean hashes become inconsistent
    uint32_t index = GetNodeIndex(node);
    if (index == UINT32_MAX) return false;
    m_nodes[index] = nullptr;
    return true;
  }

  void TrimRing() 
  { //get rid of null nodes
    std::vector<std::shared_ptr<node_t>> new_nodes_arr;
    for(auto it : m_nodes) {
      if(it != nullptr) new_nodes_arr.emplace_back(it);
    }

    m_nodes = new_nodes_arr;
  }

private:
  std::vector<std::shared_ptr<node_t>> m_nodes;
  std::hash<key_t>                     m_key_hasher;
};