#ifndef game_h
#include "../chuffed-patch/game.h"
#endif

#include "tarjan.h"
#include <vector>

TarjanSCC::TarjanSCC(Game& g) 
: g(g), indices(g.nvertices,-1), lowlink(g.nvertices,-1), onstack(g.nvertices,false) 
{        
}

//----------------------------------------------------------------------------------

std::vector<std::vector<int>> TarjanSCC::solveRAW() {
    for (int v=0; v<g.nvertices; v++) {
        if (indices[v] ==-1) {
            searchRAW(v);
        }
    }
    return sccs;
}

//----------------------------------------------------------------------------------

void TarjanSCC::searchRAW(int v) {
    indices[v] = lowlink[v] = index;
    index++;
    stack.push(v);
    onstack[v] = true;

    for(auto& e : g.outs[v]) {
        int w = g.targets[e];
        if (indices[w] == -1) {
            searchRAW(w);
            lowlink[v] = lowlink[v]<lowlink[w]?lowlink[v]:lowlink[w];
        }
        else if (onstack[w]) {
            lowlink[v] = lowlink[v]<lowlink[w]?lowlink[v]:lowlink[w];
        }
    }
    if (lowlink[v] == indices[v]) {
        std::vector<int> scc;
        while (true){
            int w = stack.top();
            stack.pop();
            onstack[w] = false;
            scc.push_back(w);
            if (w==v) break;
        }
        sccs.push_back(scc);            
    }
}

//----------------------------------------------------------------------------------

std::vector<std::vector<int>> TarjanSCC::solve() {
    for (auto& v : g.getVertices()) {
        if (indices[v] ==-1) {
            search(v);
        }
    }
    return sccs;
}

//----------------------------------------------------------------------------------

void TarjanSCC::search(int v) {
    indices[v] = lowlink[v] = index;
    index++;
    stack.push(v);
    onstack[v] = true;

    for(auto& e : g.getOuts(v)) {
        int w = g.targets[e];
        if (indices[w] == -1) {
            search(w);
            lowlink[v] = lowlink[v]<lowlink[w]?lowlink[v]:lowlink[w];
        }
        else if (onstack[w]) {
            lowlink[v] = lowlink[v]<lowlink[w]?lowlink[v]:lowlink[w];
        }
    }
    if (lowlink[v] == indices[v]) {
        std::vector<int> scc;
        while (true){
            int w = stack.top();
            stack.pop();
            onstack[w] = false;
            scc.push_back(w);
            if (w==v) break;
        }
        sccs.push_back(scc);            
    }
}

//----------------------------------------------------------------------------------
