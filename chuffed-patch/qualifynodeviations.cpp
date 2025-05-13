#ifndef game_h
#include "game.h"
#endif

#include "iostream"
#include "chuffed/vars/modelling.h"
#include "chuffed/core/propagator.h"

class QualifyNoDeviations : public Propagator {
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
    QualifyNoDeviations(Game& g, vec<BoolView>& Q)
    :   g(g), Q(Q)
    {
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
    int checker(vec<int> pathV, int v) 
    {
        int index = findVertex(v,pathV);
        if (index >= 0) {
            int best = bestcolor(index,pathV);
            bool parity = best%2;
            if (parity == g.owners[v]) {

                bool detour = false;
                for (int i=index; i<pathV.size(); i++) {
                    int v_ = pathV[i];
                    if (g.owners[v_]!=parity && g.outs[v_].size()>1) {
                        detour = true;
                        break;
                    }
                }
                if (!detour) {
                    if (!Q[v].isFixed()) {
                        vec<Lit> lits;
                        lits.push();
                        // for (int i=0; i<g.outs[v].size(); i++) {
                        //     int w = g.targets[g.outs[v][i]];
                        //     lits.push(Q[w].getLit(parity));
                        // }
                        Clause* reason = Reason_new(lits);
                        if (!Q[v].setVal( parity ,reason)) return CF_CONFLICT;
                        return CF_QUALIFIED;
                    }
                }
            }
        }
        else {
            for (auto& e : g.outs[v]) {
                int w = g.targets[e];
                if (!Q[w].isFixed()) {
                    vec<int> newpathV(pathV);
                    newpathV.push(v);
                    int status = checker(newpathV, w);
                    if (status != CF_DONE)
                        return status;
                }
            }
            
        }
        return CF_DONE;
    }
    //-----------------------------------------------------------------------
    bool propagate() override {
        for(int v=0; v<g.nvertices; v++) {
            if (!Q[v].isFixed()) {
                vec<int> pathV;
                int status = checker(pathV, v);
                if (status == CF_CONFLICT)  return false;
                if (status == CF_QUALIFIED) return true;
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
    //-----------------------------------------------------------------------
};
