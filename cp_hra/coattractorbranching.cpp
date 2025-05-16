#ifndef game_h
#include "../chuffed-patch/game.h"
#endif

#include "chuffed/branching/branching.h"
#include "chuffed/core/propagator.h" // Ensure BoolView is defined

class CoAttractorBranching : public Branching {
private:
    const int   CF_DONE     = 1;
    const int   CF_FOUND    = 2;
public:
    Game& g;
    vec<BoolView> Q;

    //-----------------------------------------------------------------------
    std::vector<int> getBestVertices(bool* removed) {
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
    void attractor(int player, std::vector<int>&U, bool* removed) {
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
    }
    //-----------------------------------------------------------------------
    std::array<std::vector<int>,3> search(bool* removed) {
        std::vector<int> A = getBestVertices(removed);
        if (A.size() == 0) {
            return { std::vector<int>(), std::vector<int>(), {CF_DONE} };
        }
        int player = g.colors[A[0]] % 2;
        std::unique_ptr<bool[]> removed1 = std::make_unique<bool[]>(g.nvertices);
        std::copy_n(removed, g.nvertices, removed1.get());

        attractor(player, A, removed1.get());
        auto win1 = search(removed1.get()); 
        if (win1[2][0] == CF_FOUND) return win1;
        if (!win1[1-player].size()) {   
            return { std::vector<int>(), std::vector<int>(), {CF_FOUND,A[0],player}};
        }
        else {
            std::unique_ptr<bool[]> removed2 = std::make_unique<bool[]>(g.nvertices);
            std::copy_n(removed, g.nvertices, removed2.get());
            std::vector<int> B(win1[1-player]);
            attractor(1-player, B, removed2.get());
            auto win2 = search(removed2.get());
            if (win2[2][0] == CF_FOUND) return win2;

            assert("Become exponential" && false);

            win2[1-player].reserve(win2[1-player].size() + B.size());
            win2[1-player].insert(win2[1-player].end(), B.begin(), B.end());
            win2[2] = {CF_DONE};
            return win2;
        }
    }

public:
    //-----------------------------------------------------------------------
    CoAttractorBranching(Game& g, vec<BoolView>& Q) : g(g), Q(Q){}
    //-----------------------------------------------------------------------
    bool finished() override {
        for(int i=0; i<Q.size(); i++) {
            if (!Q[i].finished()) return false;
        }
        return true;
    }
    //-----------------------------------------------------------------------
    DecInfo* branch() override {
        std::unique_ptr<bool[]> removed = std::make_unique<bool[]>(g.nvertices);
        std::fill_n(removed.get(), g.nvertices, false);

        for (int v=0; v<g.nvertices; v++) {
            if (Q[v].isFixed()) removed[v] = true;
        }
        auto win = search(removed.get());
        assert(win[2][0]==CF_FOUND);
        int var = win[2][1];
        int val = win[2][2];

        Q[var].setPreferredVal(val?PV_MAX:PV_MIN);
        return Q[var].branch();
    }
    //-----------------------------------------------------------------------
    double getScore(VarBranch /*vb*/) override { 
        NEVER; 
    }

};
