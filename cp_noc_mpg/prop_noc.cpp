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

// class WinningCondition {
// protected:
//     Game& g;
//     vec<BoolView> V;
//     vec<BoolView> E;
//     parity_type playerSAT;

// public:
//     WinningCondition() [

//     ]
//     WinningCondition(Game& g, vec<BoolView>& V, vec<BoolView>& E, 
//         parity_type playerSAT=EVEN) 
//     : g(g), V(V), E(E), playerSAT(playerSAT)
//     {
//     }
//     //-----------------------------------------------------------------------
//     virtual bool evaluate(int index,vec<int>& pathV,vec<int>& pathE) = 0;
// };

//===========================================================================

class NoOpponentCycle : public Propagator {
private:
    Game& g;
    vec<BoolView> V;
    vec<BoolView> E;
    int threshold;
    int filtertype;
    parity_type playerSAT;

    const int   CF_DONE     = 1;
    const int   CF_CONFLICT = 2;
    const int   CF_STAY     = 3;

public:
    //-----------------------------------------------------------------------
    NoOpponentCycle(Game& g, vec<BoolView>& V, vec<BoolView>& E, int threshold,
        int filtertype, parity_type playerSAT)
    : g(g), V(V), E(E), threshold(threshold),
        filtertype(filtertype), playerSAT(playerSAT)
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
    // Winning condition for parity games
    // bool condition(int index,vec<int>& pathV,vec<int>& pathE) {
    //     int m = g.colors[pathV[index]];
    //     for (int i=index+1; i<pathV.size(); i++) {
    //         if (g.compareColors(g.colors[pathV[i]],m,BET)) {
    //             m = g.colors[pathV[i]];
    //         }
    //     }
    //     return m%2==opponent(playerSAT);
    // }
    
    //-----------------------------------------------------------------------
    // Winning condition for energy games
    // bool condition(int index,vec<int>& pathV,vec<int>& pathE) {
    //     int sum = 0;
    //     for (int i=index; i<pathE.size(); i++) {
    //         sum += g.weights[pathE[i]];
    //     }
        
    //     if (playerSAT == EVEN) {
    //         return sum < 0; // Reject if the average benefits to opponent(EVEN)
    //     }
    //     return sum >= 0;
    // }

    //-----------------------------------------------------------------------
    // Winning condition for mean-payoff games
    bool condition(int index,vec<int>& pathV,vec<int>& pathE) {
        int sum = 0;
        for (int i=index; i<pathE.size(); i++) {
            sum += g.weights[pathE[i]];
        }
        int avg = sum / (pathE.size() - index);

        if (playerSAT == EVEN) {
            return avg < threshold; // Reject if the average benefits to opponent(EVEN)
        }
        return avg >= threshold;
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

    //-----------------------------------------------------------------------
    int filterEager(vec<int>& pathV, vec<int>& pathE, int v, 
        int lastEdge, bool definedEdge) 
    {
        int index = findVertex(v,pathV);
        if (index >= 0) {
            if (not condition(index,pathV,pathE)) {
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

        switch (filtertype) {
        case 1:
            if (filterEager(pathV,pathE,g.start,-1,true) == CF_CONFLICT)
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
