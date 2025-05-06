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

    const int   CF_DONE         = 1;
    const int   CF_CONFLICT     = 2;
    const int   CF_QUALIFIED    = 3;

public:
    //-----------------------------------------------------------------------
    Qualify(Game& g, vec<BoolView>& V, vec<BoolView>& E, vec<BoolView>& Q)
    :   g(g), V(V), E(E), Q(Q)
    {
        for (int i=0; i<g.nvertices; i++)   V[i].attach(this, 1 , EVENT_F );
        for (int i=0; i<g.nedges; i++)      E[i].attach(this, 1 , EVENT_F );
        for (int i=0; i<g.nvertices; i++)   Q[i].attach(this, 1 , EVENT_F );
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
    int checker(vec<int> path, int v, int lastEdge) 
    {
        int index = findVertex(v,path);
        if (index >= 0) {
            int best = bestcolor(index,path);
            bool parity = best%2;
            if (parity == g.owners[v] || g.vedges[v].size()==1) {
                vec<Lit> lits;
                lits.push();
                clausify(path,V,lits,0);
                Clause* reason = Reason_new(lits);
                if (!Q[v].isFixed() && !Q[v].setVal( parity ,reason)) {
                    return CF_CONFLICT;
                }
                return CF_QUALIFIED;
            }
            for (auto& e : g.vedges[v]) {
                if (E[e].isTrue()) {
                    int w = g.targets[e];
                    vec<int> path;
                    if (checker(path, g.start,-1) == CF_CONFLICT)
                        return false;
                            }
            }
        }
        else {
            for (auto& e : g.vedges[v]) {
                if (E[e].isTrue()) {
                    int w = g.targets[e];
                    if (!Q[w].isFixed()) {
                        vec<int> newpath(path);
                        newpath.push(v);
                        int status = checker(newpath, w, e);
                        return status;
                    }
                }
            }
            
        }
        return CF_DONE;
    }
    //-----------------------------------------------------------------------
    bool propagate() override {
        vec<int> path;

        if (checker(path, g.start,-1) == CF_CONFLICT)
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
