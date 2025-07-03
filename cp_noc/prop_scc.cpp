#ifndef GAME_H
#include "../various/game.h"
#endif

#ifndef TARJAN_H
#include "../various/tarjan.h"
#endif

#include "iostream"
#include "chuffed/vars/modelling.h"
#include "chuffed/core/propagator.h"
#include "stack"

class NOCSCC : public Propagator {
private:
    Game& g;
    vec<BoolView> V;
    vec<BoolView> E;
    parity_type playerSAT;

public:
    //-----------------------------------------------------------------------
    NOCSCC(Game& g, vec<BoolView>& V,vec<BoolView>& E,parity_type playerSAT) 
    : g(g), V(V), E(E), playerSAT(playerSAT)
    {
        for (int i=0; i<g.owners.size(); i++)  V[i].attach(this, 1 , EVENT_F );
        for (int i=0; i<g.sources.size(); i++) E[i].attach(this, 1 , EVENT_F );
    }
    //-----------------------------------------------------------------------
    std::vector<int> bestColors(std::vector<int> subgraph) {
        bool first = true;
        std::vector<int> best_priorities;
        for (auto& v : subgraph) {
            if (first) {
                best_priorities.push_back(v);
                first = false;
                continue;
            }
            if (g.compareVertices(v,best_priorities[0],BET)) {
                best_priorities.clear();
                best_priorities.push_back(v);
                continue;
            }
            if (g.compareVertices(v,best_priorities[0],EQU) ) {
                best_priorities.push_back(v);
                continue;
            }
        }
        return best_priorities;
    }
    //-----------------------------------------------------------------------
    bool backtrack() 
    {
        vec<Lit> lits;
        lits.push();
        int first = -1;
        for (int i=0; i<g.nvertices; i++) 
            if(V[i].isFixed()) {
                if (first <0) 
                    first = i;
                else 
                    lits.push(V[i].getValLit());
            }
        for (int i=0; i<g.nedges; i++)    
            if(E[i].isFixed()) lits.push(E[i].getValLit());

        Clause* reason = Reason_new(lits);
        V[first].setVal(V[first].isFalse(),reason);
        return false;
    }
    //-----------------------------------------------------------------------
    bool propagate() override {
        for (int i=0; i<g.nvertices; i++) {
            if (V[i].isFixed())
                g.currentv[i] = (V[i].isTrue());
            else 
                g.currentv[i] = false;
        }
        for (int i=0; i<g.nedges; i++) {
            if (E[i].isFixed()) 
                g.currente[i] = (E[i].isTrue());
            else
                g.currente[i] = false;
        }

        std::stack<std::vector<int>> stack;

        TarjanSCC tar(g);
        for (auto& s : tar.solve()) stack.push(s);

        while (stack.size()>0) {
            auto sc = stack.top();
            stack.pop();

            if (sc.size()==1) {
                int v = sc[0];
                for(auto& e : g.outs[v]) {
                    if (E[e].isFalse() || !g.currente[e]) continue;
                    int w = g.targets[e];
                    if (!g.currentv[w]) continue;
                    if (v==w && g.colors[v]%2 == opponent(playerSAT)) {
                        return backtrack();
                    }
                }
                continue;
            }

            std::vector<int> best = bestColors(sc);

            if (g.colors[best[0]]%2 == opponent(playerSAT)) {
                return backtrack();
            }

            g.deactiveAll();
            for (auto& v : sc) {
                if (std::find(best.begin(), best.end(), v) == best.end() && V[v].isTrue()) {
                    g.currentv[v] = true;
                }
            }
            for (auto& v : sc) {
                if (std::find(best.begin(), best.end(), v) == best.end()) {
                    for (auto& e : g.outs[v]) { int w = g.targets[e];
                        if ( E[e].isTrue() && g.currentv[w] && V[w].isTrue()) {
                            g.currente[e] = true;
                        }
                    }
                }
            }

            TarjanSCC tar(g);
            for (auto& s : tar.solve()) stack.push(s);
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
};
