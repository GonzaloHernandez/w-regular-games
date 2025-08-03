#ifndef GAME_H
#include "game.h"
#endif

#include "iostream"
#include "chuffed/vars/modelling.h"
#include "chuffed/core/propagator.h"

struct s_memo {
    int loop;
    int best;
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
    int checkerDFS(vec<int>& pathV, vec<int>& pathE, int v, int lastEdge) 
    {
        int index = findVertex(v,pathV);
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
            pathV.push(v);
            for (int e : g.outs[v]) {
                if (!E[e].isTrue()) continue;

                int w = g.targets[e];
                pathE.push(e);
                int status = checkerDFS(pathV, pathE, w, e);
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
    int filterEager(vec<int>& pathV, vec<int>& pathE, int v, 
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
    int filterMemo(vec<int>& pathV, vec<int>& pathE, int v, 
        int lastEdge, bool definedEdge,vec<s_memo>& memo) 
    {
        int index = findVertex(v,pathV);
        if (index >= 0) {
            int m = bestcolor(index,pathV);
            memo[lastEdge] = {v,m};
            if (m%2==opponent(playerSAT)) {
                vec<Lit> lits;
                lits.push();
                clausify(pathE, E, lits, 0);    int from;

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

                if (!memo[e].touched()) {
                    int status = filterMemo(pathV, pathE, w, e, E[e].isTrue(),memo);
                    if (status == CF_CONFLICT) return status;
                }
                else {
                    int i;
                    for (i=0; i<pathV.size()-1; i++) {
                        if (pathV[i] == memo[e].loop) {
                            int m = bestcolor(i,pathV);
                            if (g.compareColors(m,memo[e].best,BET)) {

                                memo[e].best = m;
                                int status = filterMemo(pathV, pathE, w, e, E[e].isTrue(),memo);
                                if (status == CF_CONFLICT) return status;

                            }
                            else {
                                memo[lastEdge] = memo[e];
                            }
                            break;
                        }
                    }
                    if (i == pathV.size()) {
                        memo[lastEdge] = memo[e];
                    }
                }
                pathE.pop();
            }
            pathV.pop();
        }
        return CF_DONE;
    }
    //-----------------------------------------------------------------------
    bool propagate() override {
        vec<int> pathV;
        vec<int> pathE;

        switch (filtertype) {
        case 0:
            if (checkerDFS(pathV,pathE,g.start,-1) == CF_CONFLICT)
                return false;
            break;
        case 1:
            if (filterEager(pathV,pathE,g.start,-1,true) == CF_CONFLICT)
                return false;
            break;
        case 2:
            vec<s_memo> memo(g.nedges);
            if (filterMemo(pathV,pathE,g.start,-1,true,memo) == CF_CONFLICT)
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
