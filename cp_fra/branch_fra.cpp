#ifndef game_h
#include "../chuffed-patch/game.h"
#endif

#include "chuffed/branching/branching.h"
#include "chuffed/core/propagator.h"

class FRABranching : public Branching {
private:
    const int   CF_DONE     = 1;
    const int   CF_FOUND    = 2;
public:
    Game& g;
    vec<BoolView> Q;

    //-----------------------------------------------------------------------

    int findVertex(int vertex,std::vector<int>& path) {
        for (int i=0; i<path.size(); i++) {
            if (path[i] == vertex) return i;
        }
        return -1;
    }


    //-----------------------------------------------------------------------------------------------------

    int bestcolor(Game& g, int index,const std::vector<int>& path){
        int m = g.colors[path[index]];
        for (int i=index+1; i<path.size(); i++) {
            if ((g.reward==MIN && g.colors[path[i]] < m) || (g.reward==MAX && g.colors[path[i]] > m)) {
                m = g.colors[path[i]];
            }
        }
        return m;
    }

    //-----------------------------------------------------------------------

    signed char getPlay(std::vector<int> path, int current) {
        int index = findVertex(current,path);
        if (index >= 0) {
            int best = bestcolor(g,index,path);
            return best%2;
        }
        else {
            int p = g.owners[current];
            for(auto& e : g.outs[current]) {
                std::vector <int> newpath = path;
                newpath.push_back(current);
                auto next = getPlay(newpath, g.targets[e]);
                if (next == p) {
                    return p;
                }
            }
            return 1-p;
        }
    }

public:
    //-----------------------------------------------------------------------
    FRABranching(Game& g, vec<BoolView>& Q) : g(g), Q(Q){}
    //-----------------------------------------------------------------------
    bool finished() override {
        for(int i=0; i<Q.size(); i++) {
            if (!Q[i].finished()) return false;
        }
        return true;
    }
    //-----------------------------------------------------------------------
    DecInfo* branch() override {
        int promising = -1;
        int maxins = 0;
        for (int v=0; v<g.nvertices; v++) {
            if (!Q[v].isFixed()) {
                int m = g.ins[v].size();
                if (m > maxins) {
                    promising = v;
                    maxins = m;
                }
            }
        }

        int play = getPlay({},promising);

        Q[promising].setPreferredVal( play ?PV_MAX:PV_MIN);
        return Q[promising].branch();
    }
    //-----------------------------------------------------------------------
    double getScore(VarBranch /*vb*/) override { 
        NEVER; 
    }

};
