#ifndef GAME_H
#include "../chuffed-patch/game.cpp"
#endif

#ifndef TARJAN_H
#include "../various/tarjan.h"
#endif

#include "iostream"
#include "chuffed/vars/modelling.h"
#include "chuffed/core/propagator.h"
#include "stack"

class CheckerSCC : public Propagator {
private:
    Game& g;
    vec<BoolView> V;
    vec<BoolView> E;

public:
    //-----------------------------------------------------------------------
    CheckerSCC(Game& g, vec<BoolView>& V,vec<BoolView>& E) : g(g), V(V), E(E) {
        for (int i=0; i<g.owners.size(); i++)  V[i].attach(this, 1 , EVENT_F );
        for (int i=0; i<g.sources.size(); i++) E[i].attach(this, 1 , EVENT_F );
    }
    //-----------------------------------------------------------------------
    bool backtrack() 
    {
        vec<Lit> lits;
        lits.push();
        for (int i=1; i<g.nvertices; i++)   lits.push(V[i].getValLit());
        for (int i=0; i<g.nedges; i++)      lits.push(E[i].getValLit());
        Clause* reason = Reason_new(lits);
        V[0].setVal(V[0].isFalse(),reason);
        return false;
    }
    //-----------------------------------------------------------------------
    bool propagate() override {
        for (int i=0; i<g.nvertices; i++) {
            if (!V[i].isFixed()) return true;
            g.currentv[i] = (V[i].isTrue());
        }
        for (int i=0; i<g.nedges; i++) {
            if (!E[i].isFixed()) return true;
            g.currente[i] = (E[i].isTrue());
        }

        std::stack<std::vector<int>> stack;

        TarjanSCC t1(g);
        auto ss = t1.solve();
        for (auto& s : ss) {
            stack.push(s);
        }

        while (stack.size()>0) {
            auto sc = stack.top();
            stack.pop();

            if (sc.size()==1) {
                int v = sc[0];
                for(auto& e : g.outs[v]) {
                    if (E[e].isFalse()) continue;
                    int w = g.targets[e];
                    if (v==w && g.colors[v]%2 == ODD) {
                        return backtrack();
                    }
                }
                continue;
            }

            bool first = true;
            std::vector<int> bests;
            for (auto& v : sc) {
                if (first) {
                    bests.push_back(v);
                    first = false;
                    continue;
                }
                if (g.colors[v] < g.colors[bests[0]]) {
                    bests.clear();
                    bests.push_back(v);
                    continue;
                }
                if (g.colors[v] == g.colors[bests[0]]) {
                    bests.push_back(v);
                    continue;
                }
            }
            if (g.colors[bests[0]]%2 == ODD) {
                return backtrack();
            }

            g.deactiveAll();
            for (auto& v : sc) {
                if (std::find(bests.begin(), bests.end(), v) == bests.end()) {
                    g.currentv[v] = true;
                }
            }
            for (auto& v : sc) {
                if (std::find(bests.begin(), bests.end(), v) == bests.end()) {
                    for (auto& e : g.outs[v]) { int w = g.targets[e];
                        if ( E[e].isTrue() && g.currentv[w]) {
                            g.currente[e] = true;
                        }
                    }
                }
            }

            TarjanSCC t2(g);
            auto ss = t2.solve();
            for (auto& s : ss) {
                stack.push(s);
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
};
