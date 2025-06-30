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

public:
    //-----------------------------------------------------------------------
    FRACheckerSCC(Game& g, vec<BoolView>& Q) : g(g), Q(Q) {
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
        for(parity_type PARITY : {EVEN,ODD}) { //Assessing both winning regions

            std::vector<int> region;
            for (int i=0; i<g.nvertices; i++) {
                if (!Q[i].isFixed()) return true;
                
                if (Q[i].getVal()==PARITY) region.push_back(i);

                g.currentv[i] = (Q[i].getVal()==PARITY);
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
                        int w = g.targets[e];
                        if (v==w && g.colors[v]%2 == enemy(PARITY)) {
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
                    if (g.compareVertices(v,bests[0],BET)) {
                        bests.clear();
                        bests.push_back(v);
                        continue;
                    }
                    if (g.compareVertices(v,bests[0],EQU) ) {
                        bests.push_back(v);
                        continue;
                    }
                }
                if (g.colors[bests[0]]%2 == enemy(PARITY)) {
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
                            if (g.currentv[w]) {
                                g.currente[e] = true;
                            }
                        }
                    }
                }

                TarjanSCC tar(g);
                for (auto& s : tar.solve()) stack.push(s);
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
