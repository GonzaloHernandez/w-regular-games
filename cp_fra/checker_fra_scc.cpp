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

class FRACheckerSCC : public Propagator {
private:
    Game& g;
    vec<BoolView> Q;
    GameView view;

public:
    //-----------------------------------------------------------------------
    FRACheckerSCC(Game& g, vec<BoolView>& Q) : g(g), Q(Q), view(g) {
        for (int i=0; i<g.nvertices; i++)  Q[i].attach(this, 1 , EVENT_F );
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
        for (int i=1; i<g.nvertices; i++)   lits.push(Q[i].getValLit());
        Clause* reason = Reason_new(lits);
        Q[0].setVal(Q[0].isFalse(),reason);
        return false;
    }
    //-----------------------------------------------------------------------
    bool propagate() override {
        // Evalauting regions
        for (int v=0; v<g.nvertices; v++) {
            if (!Q[v].isFixed()) return true;
            parity_type player = (parity_type)Q[v].getVal();
            if (g.colors[v] == player) {
                bool found = false;
                for (auto& e : g.outs[v]) {
                    int w = g.targets[e];
                    if (!Q[w].isFixed()) return true;
                    if (Q[w].getVal() == player) {
                        found = true;
                        break;
                    }
                }
                if (!found) return backtrack();
            }
            else {
                for (auto& e : g.outs[v]) {
                    int w = g.targets[e];
                    if (!Q[w].isFixed()) return true;
                    if (Q[w].getVal() == opponent(player)) return backtrack();
                }
            }
        }

        // Evaluating winning condition
        

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
