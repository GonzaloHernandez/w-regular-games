#ifndef game_h
#include "game.h"
#endif

#include "iostream"
#include "chuffed/vars/modelling.h"
#include "chuffed/core/propagator.h"

class CoAttractor : public Propagator {
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
    CoAttractor(Game& g, vec<BoolView>& Q)
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
    std::vector<int> getBestVertices(std::vector<bool>& removed) {
        std::vector<int> bestVertices;
        int bestColor = g.colors[Q[0].getVal()];
        for (int i=0; i<g.nvertices; i++) {
            if (removed[i]) continue;

            if (g.colors[i] == bestColor) {
                bestVertices.push_back(i);
            }
            else {
                if (g.reward==MIN && g.colors[i] < bestColor) {
                    bestColor = g.colors[i];
                }
                else if (g.reward==MAX && g.colors[i] > bestColor) {
                    bestColor = g.colors[i];
                }
                bestVertices.clear();
                bestVertices.push_back(i);
            }
        }
        return bestVertices;
    }
    //-----------------------------------------------------------------------
    std::vector<int>& attractor(int player, std::vector<bool>& removed) {
    }
    //-----------------------------------------------------------------------
    bool propagate() override {
        std::vector<bool> removed(g.nvertices, false);
        for (int v=0; v<g.nvertices; v++) {
            if (Q[v].isFixed()) removed[v] = true;
        }

        std::vector<int> attr = getBestVertices(removed);
        int player = g.colors[attr[0]] % 2;
        int opponent = 1 - player;

        std::vector<bool> removed1 = removed;

        auto& A = attractor(player, removed1);


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
