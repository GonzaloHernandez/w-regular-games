#ifndef GAME_H
#include "../chuffed-patch/game.h"
#endif

#ifndef TARJAN_H
#include "../various/tarjan.h"
#endif

#include <iostream>
#include "chuffed/vars/modelling.h"
#include "chuffed/core/propagator.h"

//-----------------------------------------------------------------------------------------------------

class ParityChecker : public Propagator {
private:
    Game& g;
    vec<BoolView> Q;

    const int   CF_DONE         = 1;
    const int   CF_CONFLICT     = 2;
    const int   CF_QUALIFIED    = 3;

public:
    //-----------------------------------------------------------------------
    ParityChecker(Game& g, vec<BoolView>& Q) : g(g),Q(Q)
    {
        for (int i=0; i<g.nvertices; i++)   Q[i].attach(this, 1 , EVENT_F );
    }
    //-----------------------------------------------------------------------
    int findVertex(int vertex,std::vector<int>& path) {
        for (int i=0; i<path.size(); i++) {
            if (path[i] == vertex) return i;
        }
        return -1;
    }
    //-----------------------------------------------------------------------
    int bestcolor(int index,std::vector<int>& path) {
        int m = g.colors[path[index]];
        for (int i=index+1; i<path.size(); i++) {
            if (g.reward==MIN && g.colors[path[i]] < m) {
                m = g.colors[path[i]];
            }
            else if (g.reward==MAX && g.colors[path[i]] > m) {
                m = g.colors[path[i]];
            }
        }
        return m;
    }

    //-----------------------------------------------------------------------------------------------------

    bool dfs(std::vector<int> path, int v, bool parity, std::vector<bool>& touched) {
        int index = findVertex(v,path);
        if (index < 0) {
            int counter=0;
            for(auto& e : g.outs[v]) {
                int w = g.targets[e];

                if (touched[w])         continue;
                if (!Q[w].isFixed())    continue;

                if (g.owners[v] != parity && Q[w].getVal() != parity) return false;
                if (g.owners[v] == parity && Q[w].getVal() != parity) continue;
                if (g.owners[v] == parity && Q[w].getVal() == parity) {
                    if (counter==1) {
                        return true;
                        touched[v] = true;
                    }
                    else counter++;
                }

                std::vector <int> newpath = path;
                newpath.push_back(v);

                if (!dfs(newpath, w, parity, touched)) return false;
            }
            touched[v] = true;
            if (g.owners[v]==parity && counter==0) return false;
        }
        return true;
    }

    //-----------------------------------------------------------------------

    std::vector<int> getBestVertices(bool threshold=false, int best=0) {
        std::vector<int> bestVertices;
        bool found = false;
        int bestColor;
        for (int i=0; i<g.nvertices; i++) {
            if (!g.active[i]) continue;

            if (threshold) {
                if ((g.reward==MIN && g.colors[i] <= best) ||
                    (g.reward==MAX && g.colors[i] >= best)) 
                {
                    continue;
                }
            }

            if (!found) {
                bestColor = g.colors[i];
                bestVertices.push_back(i);
                found = true;
                continue;
            }
            
            if (g.colors[i] == bestColor) {
                bestVertices.push_back(i);
            }
            else if (   (g.reward==MIN && g.colors[i] < bestColor) ||
                        (g.reward==MAX && g.colors[i] > bestColor)) 
            {
                bestColor = g.colors[i];
                bestVertices.clear();
                bestVertices.push_back(i);
            }
        }
        return bestVertices;
    }
    
    //-----------------------------------------------------------------------

    void attractor(int player, std::vector<int>&U) {
        std::unique_ptr<int[]> d = std::make_unique<int[]>(g.nvertices);
        std::fill_n(d.get(), g.nvertices, 0ull);

        for(auto& w : U) d[w] = 1ull;
        for(int i=0ull; i<U.size(); i++) {
            int w = U[i];
            for(auto& e : g.ins[w]) {
                int v = g.sources[e];
                if (!g.active[v]) continue;
                bool ally = g.owners[v] == player;
                if (d[v] == 0) {
                    if (ally) {
                        U.push_back(v);
                        d[v] = 1;
                    }
                    else {
                        int outbound = 0ull;
                        for(auto& e_ : g.outs[v]) {
                            if (g.active[g.targets[e_]]) outbound++;
                        }
                        d[v] = outbound;
                        if (outbound == 1) U.push_back(v);
                    }
                }
                else if (!ally && d[v] > 1) {
                    d[v] -= 1ull;
                    if (d[v] == 1) U.push_back(v);
                }
            }
        }
    }
    
    //-----------------------------------------------------------------------
    
    bool propagate() override {

        for(int v=0; v<g.nvertices; v++) if (!Q[v].isFixed()) return true;

        // Ensuring that plays never leave their winning region
        for(int v=0; v<g.nvertices; v++) {
            std::vector<bool> touched(g.nvertices,false);
            if (!dfs( {}, v, Q[v].getVal(), touched )) {
                vec<Lit> lits;
                lits.push();
                Clause* reason = Reason_new(lits);
                Q[v].setVal( !Q[v].getVal(), reason );
                return false;
            }
        }

        // Ensuring parity condition using SCC over EVEN's vertices
        {
            for (int v=0; v<g.nvertices; v++) {
                g.active[v] = (Q[v].getVal() == EVEN);
            }
            TarjanSCC tscc(g);
            auto sccs = tscc.solve();
            auto A = getBestVertices();
            for(auto& scc : sccs) {
                while (A.size()>0) {
                    int c = g.colors[A[0]];
                    attractor(EVEN, A);
                    if (A.size() == scc.size()) {
                        if (c%2 == EVEN) {
                            return true;
                        }
                        else {
                            vec<Lit> lits;
                            lits.push();
                            Clause* reason = Reason_new(lits);
                            Q[A[0]].setVal( !Q[A[0]].getVal(), reason );
                            return false;
                        }
                    }
                    A = getBestVertices(true,c);
                }
            }
        }

        // Ensuring parity condition using SCC over ODD's vertices
        {
            for (int v=0; v<g.nvertices; v++) {
                g.active[v] = (Q[v].getVal() == ODD);
            }
            TarjanSCC tscc(g);
            auto sccs = tscc.solve();
            auto A = getBestVertices();
            for(auto& scc : sccs) {
                while (A.size()>0) {
                    int c = g.colors[A[0]];
                    attractor(ODD, A);
                    if (A.size() == scc.size()) {
                        if (c%2 == ODD) {
                            return true;
                        }
                        else {
                            vec<Lit> lits;
                            lits.push();
                            Clause* reason = Reason_new(lits);
                            Q[A[0]].setVal( !Q[A[0]].getVal(), reason );
                            return false;
                        }
                    }
                    A = getBestVertices(true,c);
                }
            }
        }

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
