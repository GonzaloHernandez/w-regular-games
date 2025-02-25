#include "iostream"
#include "chuffed/vars/modelling.h"
#include "chuffed/core/propagator.h"

int filtertype = 1;

class OddCycleFilter : public Propagator {
private:
    std::vector<int> owners;
    std::vector<int> colors;
    std::vector<int> sources;
    std::vector<int> targets;
    vec<BoolView> V;
    vec<BoolView> E;
    int start;

    const int   CF_DONE     = 1;
    const int   CF_ACTIVE   = 2;
    const int   CF_CYCLE    = 3;
    const int   CF_CONFLICT = 4;

public:
    //-----------------------------------------------------------------------
    OddCycleFilter(std::vector<int>& owners,std::vector<int>& colors,
                std::vector<int>& sources,std::vector<int>& targets,
                vec<BoolView>& V,vec<BoolView>& E,int start)
    :   owners(owners), colors(colors), 
        sources(sources), targets(targets), 
        V(V), E(E), start(start) 
    {
        for (int i=0; i<owners.size(); i++)    
            V[i].attach(this, 1 , EVENT_F );
        for (int i=0; i<sources.size(); i++)    
            E[i].attach(this, 1 , EVENT_F );
        currentV();
        currentE();
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
        int min = colors[path[index]];
        for (int i=index+1; i<path.size(); i++) {
            if (colors[path[i]] < min) {
                min = colors[path[i]];
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
    void clausify_except(vec<int>& path, vec<BoolView> &B, vec<Lit>& lits,int from,int lastEdge) {
        for (int i=from; i<path.size(); i++) {
            if (path[i] != lastEdge) {
                lits.push(B[path[i]].getValLit());
            }
        }
    }
    //-----------------------------------------------------------------------
    int filter1(vec<int> pathV, vec<int> pathE, int vertex, 
        vec<BoolView> &E, int lastEdge, bool definedEdge) 
    {
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
            for (int e=0; e<sources.size(); e++) {
                if (sources[e]==vertex && !E[e].isFalse()) {
                    vec<int> newpathV(pathV);
                    vec<int> newpathE(pathE);
                    newpathV.push(vertex);
                    newpathE.push(e);
                    int status = filter1(newpathV, newpathE, 
                                    targets[e], E, e, E[e].isTrue());
                    if (status == CF_CONFLICT) {
                        return status;
                    }
                }
            }
        }
        return CF_DONE;
    }
    //-----------------------------------------------------------------------
    int filter2(vec<int> pathV, vec<int> pathE, int vertex, 
        vec<BoolView> &E, int lastEdge, bool foundBefore) 
    {
        int indexV = findVertex(vertex,pathV);
        int indexE = findEdge(lastEdge,pathE);
        if (indexV >= 0) {
            if (indexE>=indexV) {
                if (mincolor(indexV,pathV)%2==ODD) {
                    vec<Lit> lits;
                    lits.push();
                    clausify_except(pathE,E,lits,indexV,lastEdge);
                    Clause* reason = Reason_new(lits);
                    if (! E[lastEdge].setVal(false,reason)) {
                        return CF_CONFLICT;
                    }
                }
            }
            else {
                return CF_DONE;
            }
        }
        else {
            for (int e=0; e<sources.size(); e++) {
                if (sources[e]==vertex && !E[e].isFalse()) {
                    vec<int> newpathV(pathV);
                    vec<int> newpathE(pathE);
                    newpathV.push(vertex);
                    newpathE.push(e);

                    if (!foundBefore && !E[e].isTrue()) {
                        int status = filter2(newpathV, newpathE, targets[e], E, e, true);
                        if (status == CF_CONFLICT) {
                            return status;
                        }
                    }
                    else if (!foundBefore && E[e].isTrue()) {
                        int status = filter2(newpathV, newpathE, targets[e], E, e, false);
                        if (status == CF_CONFLICT) {
                            return status;
                        }
                    }
                    else if (foundBefore && E[e].isTrue()) {
                        int status = filter2(newpathV, newpathE, targets[e], E, lastEdge, true);
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
    int filter3(vec<int> pathV, vec<int> pathE, int vertex, 
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
            for (int e=0; e<sources.size(); e++) {
                if (sources[e]==vertex && !E[e].isFalse()) {
                    vec<int> newpathV(pathV);
                    vec<int> newpathE(pathE);
                    newpathV.push(vertex);
                    newpathE.push(e);
                    int status = filter3(newpathV, newpathE, 
                                    targets[e], E, e, E[e].isTrue(),touched);
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
        bool done = false;

        vec<bool> touched(V.size(),false);

        switch (filtertype) {
        case 1:
            if (filter1(pathV,pathE,start,E,-1,true) == CF_CONFLICT)
                return false;
            break;
        
        case 2:
            if (filter2(pathV,pathE,start,E,-1,false) == CF_CONFLICT)
                return false;
            break;

        case 3:
            if (filter3(pathV,pathE,start,E,-1,true,touched) == CF_CONFLICT)
                return false;
            for (int i=0; i<touched.size(); i++) {
                if (!touched[i]) {
                    if (filter3(pathV,pathE,i,E,-1,true,touched) == CF_CONFLICT)
                        return false;
                }
            }
            break;

            default:
            if (filter1(pathV,pathE,start,E,-1,true) == CF_CONFLICT)
                return false;
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