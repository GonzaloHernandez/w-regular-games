#ifndef GAME_H
#include "game.h"
#endif

#include "iostream"
#include "chuffed/vars/modelling.h"
#include "chuffed/core/propagator.h"

class WinningCondition {
protected:
    Game& g;
    vec<BoolView> V;
    vec<BoolView> E;
    parity_type playerSAT;

public:
    WinningCondition(Game& g, vec<BoolView>& V, vec<BoolView>& E, 
        parity_type playerSAT=EVEN) 
    : g(g), V(V), E(E), playerSAT(playerSAT)
    {
    }
    virtual ~WinningCondition() = default;
    //-----------------------------------------------------------------------
    virtual bool satisfy(int index,vec<int>& pathV,vec<int>& pathE) = 0;
};

//===========================================================================

class NoOpponentCycle : public Propagator {
private:
    Game& g;
    vec<BoolView> V;
    vec<BoolView> E;
    parity_type playerSAT;
    WinningCondition& condition;

    const int   CF_DONE     = 1;
    const int   CF_CONFLICT = 2;
    const int   CF_STAY     = 3;

public:
    //-----------------------------------------------------------------------
    NoOpponentCycle(Game& g, vec<BoolView>& V, vec<BoolView>& E, 
        parity_type playerSAT, WinningCondition& condition)
    : g(g), V(V), E(E),
        playerSAT(playerSAT), condition(condition)
    {
        for (int i=0; i<g.nvertices;i++) V[i].attach(this, 1 , EVENT_F );
        for (int i=0; i<g.nedges;   i++) E[i].attach(this, 1 , EVENT_F );
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
    int filterEager(vec<int>& pathV, vec<int>& pathE, int v, 
        int lastEdge, bool definedEdge) 
    {
        int index = findVertex(v,pathV);
        if (index >= 0) {
            if (not condition.satisfy(index,pathV,pathE)) {
                vec<Lit> lits;
                lits.push();
                clausify(pathE,E,lits,0);
                Clause* reason = Reason_new(lits);
                if (! E[lastEdge].setVal(false,reason)) {
                    return CF_CONFLICT;
                }
            }
        }
        else if (definedEdge) {
            pathV.push(v);
            for (int e : g.outs[v]) {
                if (E[e].isFalse()) continue;

                int w = g.targets[e];
                pathE.push(e);
                int status = filterEager(pathV, pathE, w, e, E[e].isTrue());
                pathE.pop();
                if (status == CF_CONFLICT) {
                    return status;
                }
            }
            pathV.pop();
        }
        return CF_DONE;
    }
    //-----------------------------------------------------------------------
    bool propagate() override {
        vec<int> pathV;
        vec<int> pathE;

        if (filterEager(pathV,pathE,g.start,-1,true) == CF_CONFLICT)
            return false;

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
