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
#include <unordered_set>

class CheckerSCC : public Propagator {
private:
    Game& g;
    vec<BoolView> V;
    vec<BoolView> E;
    parity_type playerSAT;

public:
    //-----------------------------------------------------------------------
    CheckerSCC(Game& g, vec<BoolView>& V,vec<BoolView>& E, parity_type playerSAT) 
    : g(g), V(V), E(E), playerSAT(playerSAT)
    {
        for (int i=0; i<g.owners.size(); i++)  V[i].attach(this, 1 , EVENT_F );
        for (int i=0; i<g.sources.size(); i++) E[i].attach(this, 1 , EVENT_F );
    }

    //-----------------------------------------------------------------------
    std::unordered_set<int> bestColors(const std::vector<int>& subgraph) {
        bool first = true;
        std::unordered_set<int> best_priorities;

        for (int v : subgraph) {
            if (first) {
                best_priorities.insert(v);
                first = false;
                continue;
            }

            int best = *best_priorities.begin(); 
            if (g.compareVertices(v, best, BET)) {
                best_priorities.clear();
                best_priorities.insert(v);
            } else if (g.compareVertices(v, best, EQU)) {
                best_priorities.insert(v);
            }
        }

        return best_priorities;
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

        GameView view(g);

        for (int i=0; i<g.nvertices; i++) {
            if (!V[i].isFixed()) return true;
            view.vs[i] = (V[i].isTrue());
        }
        for (int i=0; i<g.nedges; i++) {
            if (!E[i].isFixed()) return true;
            view.es[i] = (E[i].isTrue());
        }

        std::deque<std::vector<int>> stack;

        TarjanSCC tar(g,view);
        for (auto& s : tar.solve()) stack.push_back(std::move(s));

        while (!stack.empty()) {
            auto sc = std::move(stack.back());
            stack.pop_back();

            if (sc.size()==1) {
                int v = sc[0];
                for(int e : g.outs[v]) {
                    if (E[e].isFalse()) continue;
                    int w = g.targets[e];
                    if (v==w && g.colors[v]%2 == opponent(playerSAT)) {
                        return backtrack();
                    }
                }
                continue;
            }

            std::unordered_set<int> bestSet = bestColors(sc);

            int v0 = *bestSet.begin();
            if (g.colors[v0]%2 == opponent(playerSAT)) {
                return backtrack();
            }

            view.deactiveAll();
            for (int v : sc) {
                if (bestSet.count(v) == 0) {
                    view.vs[v] = true;
                }
            }
            for (int v : sc) {
                if (bestSet.count(v) == 0) {
                    for (int e : g.outs[v]) { 
                        int w = g.targets[e];
                        if ( E[e].isTrue() && view.vs[w]) {
                            view.es[e] = true;
                        }
                    }
                }
            }

            TarjanSCC tar(g,view);
            for (auto& s : tar.solve()) stack.push_back(std::move(s));
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
