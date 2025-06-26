#ifndef GAME_H
#include "../chuffed-patch/game.cpp"
#endif

#ifndef TARJAN_H
#include "../various/tarjan.h"
#endif

#include "iostream"
#include "chuffed/vars/modelling.h"
#include "chuffed/core/propagator.h"
#include "stack"

struct memo {
    int loop;
    int best;
    int from;
    bool touched()  { return loop>=0; }
    bool parity()   { return best%2; }
}; 

class NoOddCycle : public Propagator {
private:
    Game& g;
    vec<BoolView> V;
    vec<BoolView> E;
    int filtertype;

    const int   CF_DONE     = 1;
    const int   CF_CONFLICT = 2;
    const int   CF_STAY     = 3;

public:
    //-----------------------------------------------------------------------
    NoOddCycle(Game& g, vec<BoolView>& V,vec<BoolView>& E,int filtertype=3)
    :   g(g), V(V), E(E), filtertype(filtertype)
    {
        for (int i=0; i<g.owners.size(); i++)  V[i].attach(this, 1 , EVENT_F );
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
    int findEdge(int edge,vec<int>& path) {
        for (int i=0; i<path.size(); i++) {
            if (path[i] == edge ) return i;
        }
        return -1;
    }
    //-----------------------------------------------------------------------
    int mincolor(int index,vec<int>& path) {
        int min = g.colors[path[index]];
        for (int i=index+1; i<path.size(); i++) {
            if (g.colors[path[i]] < min) {
                min = g.colors[path[i]];
            }
        }
        return min;
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
    int backtrack() 
    {
        vec<Lit> lits;

        lits.push();

        for (int i=1; i<g.nvertices; i++)   lits.push(V[i].getValLit());
        for (int i=0; i<g.nedges; i++)      lits.push(E[i].getValLit());

        Clause* reason = Reason_new(lits);

        V[0].setVal(V[0].isFalse(),reason);

        return CF_CONFLICT;
    }
    //-----------------------------------------------------------------------
    int checker() {
        for (int i=0; i<g.nvertices; i++) {
            if (!V[i].isFixed()) return CF_DONE;
            g.currentv[i] = (V[i].isTrue());
        }
        for (int i=0; i<g.nedges; i++) {
            if (!E[i].isFixed()) return CF_DONE;
            g.currente[i] = (E[i].isTrue());
        }

        std::stack<std::vector<int>> stack;

        TarjanSCC t1(g);
        auto ss = t1.solve();
        for (auto& s : ss) {
            stack.push(s);
        }

        while (stack.size()>0) {
            auto sc = stack.top();
            stack.pop();

            if (sc.size()==1) {
                int v = sc[0];
                for(auto& e : g.outs[v]) {
                    if (E[e].isFalse()) continue;
                    int w = g.targets[e];
                    if (v==w && g.colors[v]%2 == ODD) {
                        return backtrack();
                    }
                }
                continue;
            }

            bool first = true;
            std::vector<int> bests;
            for (auto& v : sc) {
                if (first) {
                    bests.push_back(v);
                    first = false;
                    continue;
                }
                if (g.colors[v] < g.colors[bests[0]]) {
                    bests.clear();
                    bests.push_back(v);
                    continue;
                }
                if (g.colors[v] == g.colors[bests[0]]) {
                    bests.push_back(v);
                    continue;
                }
            }
            if (g.colors[bests[0]]%2 == ODD) {
                return backtrack();
            }

            g.deactiveAll();
            for (auto& v : sc) {
                if (std::find(bests.begin(), bests.end(), v) == bests.end()) {
                    g.currentv[v] = true;
                }
            }
            for (auto& v : sc) {
                if (std::find(bests.begin(), bests.end(), v) == bests.end()) {
                    for (auto& e : g.outs[v]) { int w = g.targets[e];
                        if ( E[e].isTrue() && g.currentv[w]) {
                            g.currente[e] = true;
                        }
                    }
                }
            }

            TarjanSCC t2(g);
            auto ss = t2.solve();
            for (auto& s : ss) {
                stack.push(s);
            }
        }
        return CF_DONE;
    }
    //-----------------------------------------------------------------------
    bool propagate() override {
        if (checker() == CF_CONFLICT) return false;
        return true;
    }
    //-----------------------------------------------------------------------
    int filterBasic(vec<int> pathV, vec<int> pathE, int vertex, 
        vec<BoolView> &E, int lastEdge, bool definedEdge) 
    {
        int index = findVertex(vertex,pathV);
        if (index >= 0) {
            if (mincolor(index,pathV)%2==ODD) {
                vec<Lit> lits;
                lits.push();
                clausify(pathE,E,lits,0);
                Clause* reason = Reason_new(lits);
                if (! E[lastEdge].setVal(false,reason)) {
                    return CF_CONFLICT;
                }
            }
        }
        else {
            if (definedEdge) {
                for (auto& e : g.outs[vertex]) {
                    if (!E[e].isFalse()) {
                        vec<int> newpathV(pathV);
                        vec<int> newpathE(pathE);
                        newpathV.push(vertex);
                        newpathE.push(e);
                        int status = filterBasic(newpathV, newpathE, 
                                        g.targets[e], E, e, E[e].isTrue());
                        if (status == CF_CONFLICT) {
                            return status;
                        }
                    }
                }
            }
        }
        return CF_DONE;
    }
    //-----------------------------------------------------------------------
    int filterMemo(vec<int> pathV, vec<int> pathE, int vertex, 
        vec<BoolView> &E, int lastEdge, bool definedEdge,std::vector<std::pair<int,int>>& touched) 
    {
        int index = findVertex(vertex,pathV);
        if (index >= 0) {
            int min = mincolor(index,pathV);
            touched[lastEdge].first = vertex;
            touched[lastEdge].second = min;
            if (min%2==ODD) {
                vec<Lit> lits;
                lits.push();
                // clausify(pathE,E,lits,index);
                Clause* reason = Reason_new(lits);
                if (! E[lastEdge].setVal(false,reason)) {
                    return CF_CONFLICT;
                }
            }
        }
        else {
            if (definedEdge) {

                for (auto& e : g.outs[vertex]) {
                    if (!E[e].isFalse()) {
                        vec<int> newpathV(pathV);
                        vec<int> newpathE(pathE);
                        newpathV.push(vertex);
                        newpathE.push(e);

                        if (touched[e].first < 0) {
                            int status = filterMemo(newpathV, newpathE,g.targets[e], E, e, E[e].isTrue(),touched);
                            if (status == CF_CONFLICT) {
                                return status;
                            }
                        }
                        else {
                            int i;
                            for (i=0; i<pathV.size(); i++) {
                                if (pathV[i] == touched[e].first) {
                                    int min = mincolor(i,pathV);
                                    if (min < touched[e].second) {
                                        int status = filterMemo(newpathV, newpathE,g.targets[e], E, e, E[e].isTrue(),touched);
                                        if (status == CF_CONFLICT) {
                                            return status;
                                        }
                                    }
                                    else {
                                        touched[lastEdge] = touched[e];
                                    }
                                    break;
                                }
                            }
                            if (i == pathV.size()) {
                                touched[lastEdge] = touched[e];
                            }
                        }
                    }
                }
            }
        }
        return CF_DONE;
    }
    //-----------------------------------------------------------------------
    int filterReload(vec<int> pathV, vec<int> pathE, int vertex, 
        vec<BoolView> &E, int lastEdge, bool definedEdge, std::vector<memo>& touched) 
    {
        int index = findVertex(vertex,pathV);
        if (index >= 0) {
            if (mincolor(index,pathV)%2==ODD) {
                vec<Lit> lits;
                lits.push();
                clausify(pathE,E,lits,0);
                Clause* reason = Reason_new(lits);
                if (! E[lastEdge].setVal(false,reason)) {
                    return CF_CONFLICT;
                }
            }
            else {
                return CF_DONE;
            }
        }
        else {
            if (definedEdge) {
                for (auto& e : g.outs[vertex]) {
                    if (!E[e].isFalse()) {
                        vec<int> newpathV(pathV);
                        vec<int> newpathE(pathE);
                        newpathV.push(vertex);
                        newpathE.push(e);
                        int status = filterReload(newpathV, newpathE, 
                                        g.targets[e], E, e, E[e].isTrue(),touched);
                        if (status == CF_CONFLICT) {
                            return status;
                        }
                    }
                }
            }
        }
        return CF_STAY;
    }
    //-----------------------------------------------------------------------
    int filterMultiStart(vec<int> pathV, vec<int> pathE, int vertex, 
        vec<BoolView> &E, int lastEdge, bool definedEdge, vec<bool>& touched) 
    {
        touched[vertex] = true;
        int index = findVertex(vertex,pathV);
        if (index >= 0) {
            if (mincolor(index,pathV)%2==ODD) {
                vec<Lit> lits;
                lits.push();
                clausify(pathE,E,lits,index);
                Clause* reason = Reason_new(lits);
                if (! E[lastEdge].setVal(false,reason)) {
                    return CF_CONFLICT;
                }
            }
        }
        else if (definedEdge) {

            for (auto& e : g.outs[vertex]) {
                if (!E[e].isFalse()) {
                    vec<int> newpathV(pathV);
                    vec<int> newpathE(pathE);
                    newpathV.push(vertex);
                    newpathE.push(e);
                    int status = filterMultiStart(newpathV, newpathE, 
                        g.targets[e], E, e, E[e].isTrue(),touched);
                    if (status == CF_CONFLICT || status == CF_DONE) {
                        return status;
                    }
                }
            }
        }
        return CF_DONE;
    }
    //-----------------------------------------------------------------------
    // bool propagate() override {
    //     vec<int> pathV;
    //     vec<int> pathE;

    //     switch (filtertype) {
    //     case 0: { // Checker
    //         if (checker() == CF_CONFLICT)
    //             return false;
    //         break;
    //     }
    //     case 1: { // Basic filter affecting the edge before starting the cycle
    //         if (filterBasic(pathV,pathE,g.start,E,-1,true) == CF_CONFLICT)
    //             return false;
    //         break;
    //     }
    //     case 2: { // Applying SimpleFilter starting at every other vertex
    //         vec<bool> touched(V.size(),false);
    //         if (filterMultiStart(pathV,pathE,g.start,E,-1,true,touched) == CF_CONFLICT)
    //             return false;
    //         for (int i=0; i<touched.size(); i++) {
    //             if (!touched[i]) {
    //                 if (filterMultiStart(pathV,pathE,i,E,-1,true,touched) == CF_CONFLICT)
    //                     return false;
    //             }
    //         }
    //         break;
    //     }
    //     case 3: { // Remembering best plays
    //         std::vector<std::pair<int,int>> touched(g.nedges,{-1,-1});
    //         if (filterMemo(pathV,pathE,g.start,E,-1,true,touched) == CF_CONFLICT)
    //             return false;
    //         break;
    //     }
    //     case 4: { // new Remembering best plays
    //         std::vector<memo> touched(g.nedges,{-1,-1,-1});
    //         if (filterReload(pathV,pathE,g.start,E,-1,true,touched) == CF_CONFLICT)
    //             return false;
    //         break;
    //     }
    //     default:
    //         if (filterBasic(pathV,pathE,g.start,E,-1,true) == CF_CONFLICT)
    //             return false;
    //     }

    //     return true;
    // }
    //-----------------------------------------------------------------------
    void wakeup(int i, int) override {
        pushInQueue();
    }
    //-----------------------------------------------------------------------
    void clearPropState() override {
        in_queue = false;
    }
    //-----------------------------------------------------------------------
    std::string currentV() {
        std::stringstream out;
        out << "V=[";
        for (int i=0; i<V.size(); i++) {
            // out << i << ":";
            if (V[i].isFixed())
                out << (int)V[i].isTrue() << (i<V.size()-1?",":"");
            else
                out << " " << (i<V.size()-1?",":"");
        }
        out << "]";
        return out.str();
    }
    //-----------------------------------------------------------------------
    std::string currentE() {
        std::stringstream out;
        out << "E=[";
        for (int i=0; i<E.size(); i++) {
            // out << i << ":";
            if (E[i].isFixed())
                out << (int)E[i].isTrue() << (i<E.size()-1?",":"");
            else
                out << " " << (i<E.size()-1?",":"");
        }
        out << "]";
        return out.str();
    }
};
