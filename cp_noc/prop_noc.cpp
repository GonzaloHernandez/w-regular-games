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
    int bestcolor(int index,vec<int>& path,int v) {
        int m = g.colors[v];
        for (int i=index; i<path.size(); i++) {
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
    void clausify(vec<int>& path, vec<BoolView> &B, vec<Lit>& lits) {
        for (int i=0; i<path.size(); i++) {
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
    int checker(vec<int>& pathV, vec<int>& pathE, int v, int lastEdge) 
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
                int status = checker(pathV, pathE, w, e);
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

    int filterEagerStack() {
        vec<int>                pathV;
        vec<int>                pathE;
        std::unique_ptr<int[]>  es = std::make_unique<int[]>(g.nvertices);
        int                     v,e;
        //----------------------------------------------------------
        auto nextEdge = [&](int v_) -> int {
            int limit = g.outs[v_].size();
            while(es[v_] < limit) {
                int e_ = g.outs[v_][es[v_]];
                es[v_]++;
                if (E[e_].isFalse())            continue;
                if (pathE.size()==0)            return e_;
                if (!E[pathE.last()].isFixed()) return -1;
                if (E[e_].isTrue())             return e_;
            }
            return -1;
        };
        //----------------------------------------------------------
        auto dfsWalk = [&]() -> int {
            int c;
            while (true) {
                pathV.push(v);
                pathE.push(e);
                if (e < 0) break;
                c = findVertex(g.targets[e], pathV);
                if (c >=0) break;
                v = g.targets[e];
                es[v] = 0;
                e = nextEdge(v);
            }
            if (e>=0) {
                if (bestcolor(c,pathV,v)%2 == opponent(playerSAT)) {
                    vec<Lit> lits;
                    lits.push();
                    clausify(pathE,E,lits,0);
                    Clause* reason = Reason_new(lits);
                    if (! E[e].setVal(false,reason)) {
                        return CF_CONFLICT;
                    }
                }
            }
            return CF_DONE;
        };
        //----------------------------------------------------------

        v = g.start;
        e = nextEdge(v);

        if (dfsWalk() == CF_CONFLICT) return CF_CONFLICT;
 
        while (pathV.size()>0) {
            v = pathV.last();
            e = pathE.last();

            e = nextEdge(v);
            if (e < 0) continue;
            
            if (dfsWalk() == CF_CONFLICT) return CF_CONFLICT;
        }
        return CF_DONE;
    }

    //-----------------------------------------------------------------------
    int filterMemo(vec<int>& pathV, vec<int>& pathE, int v, 
        int lastEdge, bool definedEdge,s_memo* memo) 
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
            // if (checker(pathV,pathE,g.start,-1) == CF_CONFLICT)
            if (filterEagerStack() == CF_CONFLICT)
                return false;
            break;
        case 1:
            // if (filterEagerStack() == CF_CONFLICT) 
            if (filterEager(pathV,pathE,g.start,-1,true) == CF_CONFLICT)
                return false;
            break;
        case 2:
            std::unique_ptr<s_memo[]> memo(new s_memo[g.nedges]);
            std::fill_n(memo.get(), g.nedges, s_memo{-1,-1});
            if (filterMemo(pathV,pathE,g.start,-1,true,memo.get()) == CF_CONFLICT)
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
