#ifndef game_h
#include "../chuffed-patch/game.h"
#endif

#include "iostream"
#include "chuffed/vars/modelling.h"
#include "chuffed/core/propagator.h"

class Zielonka {
private:
    Game& g;
public:
    //-----------------------------------------------------------------------
    Zielonka(Game& g) : g(g) {
    }
    //-----------------------------------------------------------------------
    std::vector<int> getBestVertices(std::vector<bool>& removed) {
        std::vector<int> bestVertices;
        bool found = false;
        int bestColor;
        for (int i=0; i<g.nvertices; i++) {
            if (removed[i]) continue;

            if (!found) {
                bestColor = g.colors[i];
                bestVertices.push_back(i);
                found = true;
                continue;
            }
            
            if (g.colors[i] == bestColor) {
                bestVertices.push_back(i);
            }
            else if (g.reward==MIN && g.colors[i] < bestColor) {
                bestColor = g.colors[i];
                bestVertices.clear();
                bestVertices.push_back(i);
                }
            else if (g.reward==MAX && g.colors[i] > bestColor) {
                bestColor = g.colors[i];
                bestVertices.clear();
                bestVertices.push_back(i);
            }
        }
        return bestVertices;
    }
    //-----------------------------------------------------------------------
    std::vector<int> attractor(int player, std::vector<int>U, std::vector<bool>& removed) {
        std::vector<int> d(g.nvertices,0);
        for(auto& w : U) d[w] = 1;
        for(int i=0; i<U.size(); i++) {
            int w = U[i];
            for(auto& e : g.ins[w]) {
                int v = g.sources[e];
                if (removed[v]) continue;
                bool ally = g.owners[v] == player;
                if (d[v] == 0) {
                    if (ally) {
                        U.push_back(v);
                        d[v] = 1;
                    }
                    else {
                        int outbound = 0;
                        for(auto& e_ : g.outs[v]) {
                            if (!removed[g.targets[e_]]) outbound++;
                        }
                        d[v] = outbound;
                        if (outbound == 1) U.push_back(v);
                    }
                }
                else if (!ally && d[v] > 1) {
                    d[v] -= 1;
                    if (d[v] == 1) U.push_back(v);
                }
            }
        }
        for (auto& w : U) {
            removed[w] = true;
        }
        return U;
    }
    //-----------------------------------------------------------------------
    std::array<std::vector<int>,2> search(std::vector<bool>& removed) {
        std::vector<int> U = getBestVertices(removed);
        if (U.size() == 0) {
            return { std::vector<int>(), std::vector<int>() };
        }
        int player = g.colors[U[0]] % 2;
        std::vector<bool> removed1 = removed;
        auto A = attractor(player, U, removed1);
        auto win1 = search(removed1); 
        if (!win1[1-player].size()) {
            win1[player].reserve(win1[player].size() + A.size());
            win1[player].insert(win1[player].end(), A.begin(), A.end());
            return win1;
        }
        else {
            std::vector<bool> removed2 = removed;
            auto B = attractor(1-player, win1[1-player], removed2);
            auto win2 = search(removed2);
            win2[1-player].reserve(win2[1-player].size() + B.size());
            win2[1-player].insert(win2[1-player].end(), B.begin(), B.end());
            return win2;
        }
    }
    //-----------------------------------------------------------------------
    std::array<std::vector<int>,2> solve() {
        std::vector<bool> removed(g.nvertices, false);
        return search(removed);
    }
};
