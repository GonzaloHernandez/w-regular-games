#include "graph.h"
#include <algorithm>
#include <ctype.h>
#include <fstream>
#include <sstream>
#include <string>

void Graph::Attract(std::vector<size_t>* attr, uint8_t player, bool * removed)
{
    // Mirrors impl at https://github.com/umbertomarotta/SPGSolver/blob/master/Java/src/main/java/com/JPGSolver/RecursiveSolver.java

    // A is vector that contains all attracted nodes
    std::vector<size_t>& attracted = *attr;
    
    // Create tmp_map and fill with 1
    std::unique_ptr<size_t[]> tmp_map = std::make_unique<size_t[]>(n_nodes_);
    std::fill_n(tmp_map.get(), n_nodes_, 0ull);
    // Set 1 for anything in the attracted
    for (auto& v : attracted)
    {
        tmp_map[v] = 1ull;
    }

    for (size_t count = 0ull; count < attracted.size(); count++)
    {
        for (size_t& v0 : nodes_[attracted[count]].inbound)
        {
            // v0 will be the sources of inbound edges to node with index attracted[count]
            // Check v0 is not removed
            if (removed[v0])
            {
                continue;
            }
            // Check the node is on our (player's) team
            bool ally = nodes_[v0].owner == player;
            if (tmp_map[v0] == 0)
            {
                if (ally)
                {
                    attracted.push_back(v0);
                    tmp_map[v0] = 1;
                }
                else
                {
                    // See the degree of freedom for this enemy node (number of outbound edges)
                    size_t outbounds = 0ull;
                    for (auto& v : nodes_[v0].outbound)
                    {
                        if (!removed[v])
                        {
                            outbounds++;
                        }
                    }
                    tmp_map[v0] = outbounds;
                    if (outbounds == 1)
                    {
                        attracted.push_back(v0);
                    }
                }
            }
            else if (!ally && tmp_map[v0] > 1)
            {
                tmp_map[v0] -= 1ull;
                if (tmp_map[v0] == 1)
                {
                    attracted.push_back(v0);
                }
            }
        }
    }

    // Remove the attracted nodes
    for (size_t& v : attracted)
    {
        removed[v] = true;
    }
}

std::vector<size_t> Graph::GetNodesWithMaxPriority(bool* removed)
{
    std::vector<size_t> res;

    size_t max_priority = 0ull;
    for (size_t i = 0ull; i < n_nodes_; i++)
    {
        if (removed[i])
        {
            continue;
        }
        if (nodes_[i].priority == max_priority)
        {
            res.push_back(i);
        }
        else if (nodes_[i].priority > max_priority)
        {
            max_priority = nodes_[i].priority;
            res.clear();
            res.push_back(i);
        }
    }

    return res;
}

std::array<std::vector<size_t>, 2> Graph::Solve(bool * removed)
{
    // Returns ([wins for player 0], [wins for player 1])
    // Just your regular Zielonka
    std::vector<size_t> attr = GetNodesWithMaxPriority(removed);
    if (!attr.size())
    {
        return { std::vector<size_t>(), std::vector<size_t>() };
    }
    uint8_t player = nodes_[attr[0]].priority % 2;
    uint8_t opponent = 1 - player;
    
    // Copy removed array
    std::unique_ptr<bool[]> removed1 = std::make_unique<bool[]>(n_nodes_);
    std::copy_n(removed, n_nodes_, removed1.get());

    Attract(&attr, player, removed1.get());
    if (false) {
        printf("(1) %zu | ", attr.size());
        for (size_t i = 0ull; i < attr.size(); i++)
        {
            printf("%zu ", attr[i]);
        }
        printf("\n");
    }
    auto win1 = Solve(removed1.get());
    if (!win1[opponent].size())
    {
        win1[player].reserve(win1[player].size() + attr.size());
        win1[player].insert(win1[player].end(), attr.begin(), attr.end());
        return win1;
    }
    else
    {
        // Copy removed array again
        std::unique_ptr<bool[]> removed2 = std::make_unique<bool[]>(n_nodes_);
        std::copy_n(removed, n_nodes_, removed2.get());

        std::vector<size_t> _attr(win1[opponent]);
        Attract(&_attr, opponent, removed2.get());
        if (false) {
            printf("(2) %zu | ", _attr.size());
            for (size_t i = 0ull; i < _attr.size(); i++)
            {
                printf("%zu ", _attr[i]);
            }
            printf("\n");
        }
        auto win2 = Solve(removed2.get());

        win2[opponent].reserve(win2[opponent].size() + _attr.size());
        win2[opponent].insert(win2[opponent].end(), _attr.begin(), _attr.end());
        return win2;
    }
}

Graph::Graph(const char * fname)
{
    std::ifstream input(fname);
    /*
    File contents like: (line breaks are optional)
        parity 7;
        start 0; (optional line we will ignore)
        0 0 1 1,2,3;
        1 1 0 4;
        2 983042 0 4;
        3 1 0 5;
        4 0 0 4;
        5 1 1 1,6;
        6 1 0 7;
        7 1 1 1,3;

    Columns are: INDEX(sorted and 0 indexed), PRIORITY, OWNER(0 or 1), OUTBOUND_DESTINATIONS(comma separated)

    Also note that nodes may be specified as:
        0 0 1 1,2,3 "comment";
    */
    size_t n_nodes;
    std::string line;
    std::getline(input, line, ';');
    std::istringstream(line) >> line >> n_nodes;
    n_nodes++;
    n_nodes_ = n_nodes;

    // Initialise nodes
    nodes_ = std::make_unique<Node[]>(n_nodes);
    for (size_t i = 0ull; i < n_nodes; i++)
    {
        nodes_[i] = { 0ull, 0ull, (uint8_t)0, std::vector<size_t>(), std::vector<size_t>() };
    }

    while (input.good())
    {
        std::getline(input, line, ';');
        // Trim leading whitespace
        auto trim = line.find_first_not_of(" \n");
        if (trim == std::string::npos)
        {
            continue;
        }
        line = line.substr(line.find_first_not_of(" \n"));

        // Check leading character is a digit
        if (!isdigit(line.at(0)))
        {
            continue;
        }

        size_t index, priority;
        int owner;

        std::istringstream ss(line);
        ss >> index >> priority >> owner;
        nodes_[index].index = index;
        nodes_[index].priority = priority;
        nodes_[index].owner = (uint8_t)owner;

        // Read comma-separated destinations until <doublequote> or end
        while (ss.good())
        {
            size_t dest;
            ss >> dest;
            nodes_[index].outbound.push_back(dest);
            nodes_[dest].inbound.push_back(index);

            std::string garbage;
            std::getline(ss, garbage, ',');
        }
    }
}

std::pair<std::vector<size_t>, std::vector<size_t>> Graph::Solve()
{
    // Create a bool[] of all false because nothing has been excluded yet
    // use bool[] because lookup performance may be more important than copy performance
    // also simpler than bitset
    std::unique_ptr<bool[]> removed = std::make_unique<bool[]>(n_nodes_);
    std::fill_n(removed.get(), n_nodes_, false);

    auto res = Solve(removed.get());
    return { res[0], res[1] };
}
