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

struct memo {
    int loop;
    int best;
    int from;
    bool touched()  { return loop>=0; }
    bool parity()   { return best%2; }
}; 

class NoOpponentCycle : public Propagator {
private:
    Game& g;
    vec<BoolView> E;
    int filtertype;
    parity_type playerSAT;

    const int   CF_DONE     = 1;
    const int   CF_CONFLICT = 2;
    const int   CF_STAY     = 3;

public:
    //-----------------------------------------------------------------------
    NoOpponentCycle(Game& g, vec<BoolView>& E, int filtertype, parity_type playerSAT)
    :   g(g), E(E), filtertype(filtertype), playerSAT(playerSAT)
    {
        for (int i=0; i<g.sources.size(); i++) E[i].attach(this, 1 , EVENT_F );
    }
    //-----------------------------------------------------------------------
    int findVertex(int vertex,vec<int>& path) {
        for (int i=0; i<path.size(); i++) {
            if (path[i] == vertex) return i;
        }
        return -1;
    }
    //-----------------------------------------------------------------------
    int findEdge(int edge,vec<int>& path) {
        for (int i=0; i<path.size(); i++) {
            if (path[i] == edge ) return i;
        }
        return -1;
    }
    //-----------------------------------------------------------------------
    int bestcolor(int index,vec<int>& path) {
        int m = g.colors[path[index]];
        for (int i=index+1; i<path.size(); i++) {
            if (g.compareColors(g.colors[path[i]],m,BET)) {
                m = g.colors[path[i]];
            }
        }
        return m;
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
    int checker(vec<int> pathV, vec<int> pathE, int vertex, int lastEdge) 
    {
        int index = findVertex(vertex,pathV);
        if (index >= 0) {
            if (bestcolor(index,pathV)%2==opponent(playerSAT)) {
                vec<Lit> lits;
                lits.push();
                clausify(pathE,E,lits,index);
                Clause* reason = Reason_new(lits);
                if (! E[lastEdge].setVal(false,reason)) {
                    return CF_CONFLICT;
                }
            }
        }
        else {
            for (auto& e : g.outs[vertex]) {
                if (E[e].isTrue()) {
                    vec<int> newpathV(pathV);
                    vec<int> newpathE(pathE);
                    newpathV.push(vertex);
                    newpathE.push(e);
                    int status = checker(newpathV, newpathE, g.targets[e], e);
                    if (status == CF_CONFLICT) {
                        return status;
                    }
                }
            }
        }
        return CF_DONE;
    }
    //-----------------------------------------------------------------------
    int filterBasic(vec<int> pathV, vec<int> pathE, int v, 
        int lastEdge, bool definedEdge) 
    {
        int index = findVertex(v,pathV);
        if (index >= 0) {
            if (bestcolor(index,pathV)%2==opponent(playerSAT)) {
                vec<Lit> lits;
                lits.push();
                clausify(pathE,E,lits,0);
                Clause* reason = Reason_new(lits);
                if (! E[lastEdge].setVal(false,reason)) {
                    return CF_CONFLICT;
                }
            }
        }
        else {
            if (definedEdge) {
                for (auto& e : g.outs[v]) {
                    if (!E[e].isFalse()) {
                        vec<int> newpathV(pathV);
                        vec<int> newpathE(pathE);
                        newpathV.push(v);
                        newpathE.push(e);
                        int status = filterBasic(newpathV, newpathE, 
                                        g.targets[e], e, E[e].isTrue());
                        if (status == CF_CONFLICT) {
                            return status;
                        }
                    }
                }
            }
        }
        return CF_DONE;
    }
    //-----------------------------------------------------------------------
    int filterMemo(vec<int> pathV, vec<int> pathE, int v, 
        int lastEdge, bool definedEdge,std::vector<std::pair<int,int>>& touched) 
    {
        int index = findVertex(v,pathV);
        if (index >= 0) {
            int m = bestcolor(index,pathV);
            touched[lastEdge].first = v;
            touched[lastEdge].second = m;
            if (m%2==opponent(playerSAT)) {
                vec<Lit> lits;
                lits.push();
                clausify_except(pathE, E, lits, 0, lastEdge);
                Clause* reason = Reason_new(lits);
                if (! E[lastEdge].setVal(false,reason)) {
                    return CF_CONFLICT;
                }
            }
        }
        else {
            if (definedEdge) {

                for (auto& e : g.outs[v]) {
                    if (!E[e].isFalse()) {
                        vec<int> newpathV(pathV);
                        vec<int> newpathE(pathE);
                        newpathV.push(v);
                        newpathE.push(e);

                        if (touched[e].first < 0) {
                            int status = filterMemo(newpathV, newpathE,g.targets[e], e, E[e].isTrue(),touched);
                            if (status == CF_CONFLICT) {
                                return status;
                            }
                        }
                        else {
                            int i;
                            for (i=0; i<pathV.size(); i++) {
                                if (pathV[i] == touched[e].first) {
                                    int m = bestcolor(i,pathV);
                                    if (g.compareColors(m,touched[e].second,BET)) {
                                        int status = filterMemo(newpathV, newpathE,g.targets[e], e, E[e].isTrue(),touched);
                                        if (status == CF_CONFLICT) {
                                            return status;
                                        }
                                    }
                                    else {
                                        touched[lastEdge] = touched[e];
                                    }
                                    break;
                                }
                            }
                            if (i == pathV.size()) {
                                touched[lastEdge] = touched[e];
                            }
                        }
                    }
                }
            }
        }
        return CF_DONE;
    }
    //-----------------------------------------------------------------------
    bool propagate() override {
        vec<int> pathV;
        vec<int> pathE;

        switch (filtertype) {
        case 0:
            if (checker(pathV,pathE,g.start,-1) == CF_CONFLICT)
                return false;
            break;
        case 1:
            if (filterBasic(pathV,pathE,g.start,-1,true) == CF_CONFLICT)
                return false;
            break;
        case 2:
            std::vector<std::pair<int,int>> touched(g.nedges,{-1,-1});
            if (filterMemo(pathV,pathE,g.start,-1,true,touched) == CF_CONFLICT)
                return false;
            break;
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
