#pragma once
#include <cstddef>
#include <memory>
#include <array>
#include <vector>

struct Node
{
    size_t index;
    size_t priority;
    uint8_t owner;
    // Contains node indexes of outbound destinations
    std::vector<size_t> outbound;
    // Contains node indexes of inbound nodes with this node as an outbound destination
    std::vector<size_t> inbound;
};

class Graph
{
protected:
    size_t start_;
    size_t n_nodes_;
    std::unique_ptr<Node[]> nodes_;

    virtual void Attract(std::vector<size_t>* attr, uint8_t player, bool* removed);
    std::vector<size_t> GetNodesWithMaxPriority(bool* removed);
    std::array<std::vector<size_t>, 2> Solve(bool* removed);

public:
    Graph(const char* fname);

    size_t n_nodes() { return n_nodes_; }

    std::pair<std::vector<size_t>, std::vector<size_t>> Solve();
};
