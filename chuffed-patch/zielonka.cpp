#ifndef game_h
#include "game.h"
#endif

#include "iostream"
#include "chuffed/vars/modelling.h"
#include "chuffed/core/propagator.h"

class Zielonka : public Propagator {
private:
    Game& g;
    vec<BoolView> V;
    vec<BoolView> E;
    vec<BoolView> Q;

    const int   CF_DONE         = 1;
    const int   CF_CONFLICT     = 2;
    const int   CF_QUALIFIED    = 3;

public:
    //-----------------------------------------------------------------------
    Zielonka(Game& g, vec<BoolView>& Q)
    :   g(g), Q(Q)
    {
        priority = 5;
        for (int i=0; i<g.nvertices; i++)   Q[i].attach(this, 1 , EVENT_F );
    }
    //-----------------------------------------------------------------------
    int findVertex(int vertex,vec<int>& path) {
        for (int i=0; i<path.size(); i++) {
            if (path[i] == vertex) return i;
        }
        return -1;
    }
    //-----------------------------------------------------------------------
    void clausify(vec<int>& path, vec<BoolView> &B, vec<Lit>& lits,int from) {
        for (int i=from; i<path.size()-1; i++) {
            lits.push(B[path[i]].getValLit());
        }
    }
    //-----------------------------------------------------------------------
    void clausify_except(   vec<int>& path, vec<BoolView> &B, vec<Lit>& lits,
                            int from,int lastEdge) 
    {
        for (int i=from; i<path.size(); i++) {
            if (path[i] != lastEdge) {
                lits.push(B[path[i]].getValLit());
            }
        }
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
    bool propagate() override {

        std::vector<bool> removed(g.nvertices, false);
        for (int v=0; v<g.nvertices; v++) {
            if (Q[v].isFixed()) removed[v] = true;
        }
        auto win1 = search(removed);
        return true;
    }
    //-----------------------------------------------------------------------
    void wakeup(int i, int) override {
        pushInQueue();
    }
    //-----------------------------------------------------------------------
    void clearPropState() override {
        in_queue = false;
    }
    //-----------------------------------------------------------------------
};
