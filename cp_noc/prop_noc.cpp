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
    vec<BoolView> V;
    vec<BoolView> E;
    int filtertype;
    parity_type playerSAT;

    const int   CF_DONE     = 1;
    const int   CF_CONFLICT = 2;
    const int   CF_STAY     = 3;

public:
    //-----------------------------------------------------------------------
    NoOpponentCycle(Game& g, vec<BoolView>& V, vec<BoolView>& E, int filtertype, parity_type playerSAT)
    :   g(g), V(V), E(E), filtertype(filtertype), playerSAT(playerSAT)
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
    int bestcolor(int index,vec<int>& path) {
        int m = g.priors[path[index]];
        for (int i=index+1; i<path.size(); i++) {
            if (g.comparePriorities(g.priors[path[i]],m,BET)) {
                m = g.priors[path[i]];
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

    struct state{
        int v;
        int i;
    };

    int checkerDFSIterative(int v) {
        vec<int>    pathV;
        vec<int>    pathE;
        vec<state>  stack;
        stack.push({v,0});
        int e = -1;
        while (stack.size()>0) {
            state& s = stack.last();
            if (s.i == 0) {
                int index = findVertex(s.v,pathV);
                if (index >= 0) {
                    if (bestcolor(index,pathV)%2==opponent(playerSAT)) {
                        if (pathE.size()>0) {
                            vec<Lit> lits;
                            lits.push();
                            clausify(pathE,E,lits,0);
                            Clause* reason = Reason_new(lits);
                            if (! E[e].setVal(false,reason)) {
                                return CF_CONFLICT;
                            }
                        }
                    }
                    stack.pop();
                    if (pathE.size()>0) pathE.pop();
                    continue;
                }
                pathV.push(s.v);
            }

            if (s.i < g.outs[s.v].size()) {
                e = g.outs[s.v][s.i++];
                if (!E[e].isTrue()) continue;

                int w = g.targets[e];
                pathE.push(e);
                stack.push({w,0});
                continue;
            }
            else {
                if (pathE.size()>0) pathE.pop();
            }
            stack.pop();
            if (pathV.size()>0) pathV.pop();
        }
        return CF_DONE;
    }
    //-----------------------------------------------------------------------
    int checkerDFSRecursive(vec<int>& pathV, vec<int>& pathE, int v, int lastEdge) 
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
                int status = checkerDFSRecursive(pathV, pathE, w, e);
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
                            if (g.comparePriorities(m,memo[e].best,BET)) {

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
    void clausifyExcept(vec<int>& path, vec<BoolView> &B, vec<Lit>& lits,int edgeIgnored) {
        for (int i=0; i<path.size(); i++) { 
            if (path[i] == edgeIgnored) continue;
            lits.push(B[path[i]].getValLit());
        }
    }
    //-----------------------------------------------------------------------
    int filterSmart(vec<int>& pathV, vec<int>& pathE, int v, 
        int lastEdge, int undefinedEdge) 
    {
        int index = findVertex(v,pathV);
        if (index >= 0) {
            if (bestcolor(index,pathV)%2==opponent(playerSAT)) {
                vec<Lit> lits;
                lits.push();
                if (undefinedEdge == -1) {
                    clausifyExcept(pathE,E,lits,lastEdge);
                    Clause* reason = Reason_new(lits);
                    if (! E[lastEdge].setVal(false,reason)) {
                        return CF_CONFLICT;
                    }
                }
                else {
                    clausifyExcept(pathE,E,lits,undefinedEdge);
                    Clause* reason = Reason_new(lits);
                    if (! E[undefinedEdge].setVal(false,reason)) {
                        return CF_CONFLICT;
                    }
                }
            }
        }
        else {
            pathV.push(v);
            for (int e : g.outs[v]) {
                if (E[e].isFalse()) continue;
                if (!E[e].isFixed() && undefinedEdge >= 0) break;
                if (!E[e].isFixed()) undefinedEdge = e;

                int w = g.targets[e];
                pathE.push(e);
                int status = filterSmart(pathV, pathE, w, e, undefinedEdge);
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

        switch (filtertype) {
        case 0:
            if (checkerDFSRecursive(pathV,pathE,g.start,-1) == CF_CONFLICT)
                return false;
            break;
        case 1:
            if (filterEager(pathV,pathE,g.start,-1,true) == CF_CONFLICT)
                return false;
            break;
        case 2: {
            std::unique_ptr<s_memo[]> memo(new s_memo[g.nedges]);
            std::fill_n(memo.get(), g.nedges, s_memo{-1,-1});
            if (filterMemo(pathV,pathE,g.start,-1,true,memo.get()) == CF_CONFLICT)
                return false;
            break;
        }
        case 3:
            if (filterSmart(pathV,pathE,g.start,-1,-1) == CF_CONFLICT)
                return false;
            break;
        case 4:
            if (checkerDFSIterative(g.start) == CF_CONFLICT)
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
