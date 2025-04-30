#ifndef game_h
#include "game.h"
#endif

#include "iostream"
#include "chuffed/vars/modelling.h"
#include "chuffed/core/propagator.h"

class Qualify : public Propagator {
private:
    Game& g;
    vec<BoolView> V;
    vec<BoolView> E;
    vec<BoolView> Q;
    vec<BoolView> L;
    vec<BoolView> C;

    const int   CF_DONE     = 1;
    const int   CF_CONFLICT = 2;

public:
    //-----------------------------------------------------------------------
    Qualify(Game& g, 
            vec<BoolView>& V,vec<BoolView>& E,
            vec<BoolView>& Q,vec<BoolView>& L,vec<BoolView>& C)
    :   g(g), V(V), E(E), Q(Q), L(L), C(C)
    {
        for (int i=0; i<g.nvertices; i++)   V[i].attach(this, 1 , EVENT_F );
        for (int i=0; i<g.nedges; i++)      E[i].attach(this, 1 , EVENT_F );
        for (int i=0; i<g.nvertices; i++)   Q[i].attach(this, 1 , EVENT_F );
        for (int i=0; i<g.nedges; i++)      L[i].attach(this, 1 , EVENT_F );
        for (int i=0; i<g.nedges; i++)      C[i].attach(this, 1 , EVENT_F );
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
            if (g.reward==MIN && g.colors[path[i]] < m) {
                m = g.colors[path[i]];
            }
            else if (g.reward==MAX && g.colors[path[i]] > m) {
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
            int best = bestcolor(index,pathV);

            vec<Lit> lits;
            lits.push();
            clausify(pathV,V,lits,0);
            Clause* reason = Reason_new(lits);
            if (!C[lastEdge].isFixed() && !C[lastEdge].setVal( true ,reason)) {
                return CF_CONFLICT;
            }
            for (int e=0; e<pathE.size(); e++) {
                if (pathE[e] != lastEdge) {
                    if ( !C[pathE[e]].isFixed() && !C[pathE[e]].setVal( false ,reason)) {
                        return CF_CONFLICT;
                    }
                }
            }
            if (!L[lastEdge].isFixed() && !L[lastEdge].setVal( best%2 ,reason)) {
                return CF_CONFLICT;
            }
        }
        else {
            for (auto& e : g.vedges[vertex]) {
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
    bool propagate() override {
        vec<int> pathV;
        vec<int> pathE;

        if (checker(pathV,pathE,g.start,-1) == CF_CONFLICT)
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
    //-----------------------------------------------------------------------
};
