#ifndef CPP_GAME
#include "game.cpp"
#endif

#include "iostream"
#include "chuffed/vars/modelling.h"
#include "chuffed/core/propagator.h"
#include "initializer_list"
#include "chuffed/globals/dconnected.h"

#include "oddcyclefilter.cpp"

class CPModel : public Problem {
public:
    static const int DZN    = 0;
    static const int GM     = 1;

private:
    Game& g;
    vec<BoolView> V;  
    vec<BoolView> E;
    int filtertype;
    int printtype;

public:

    CPModel(Game& g,int filtertype=1,int reachability=1,int printtype=1) 
    : g(g), filtertype(filtertype), printtype(printtype) 
    {
        V.growTo(g.nvertices);
        E.growTo(g.nedges);
        setupConstraints(reachability);
    }

    //----------------------------------------------------------------

    void setupConstraints(int reachability=1) {

        for (int i=0; i<g.nvertices;  i++) V[i] = newBoolVar();
        for (int i=0; i<g.nedges;     i++) E[i] = newBoolVar();

        // Starting vertex
        fixVertices({g.start},{});

        // For every EVEN vertice, at least one outgoing edge must be activated
        for (int v=0; v<g.nvertices; v++) if (g.owners[v] == EVEN) {
            vec<Lit> clause;
            clause.push( V[v].getLit(false) );
            for (int e=0; e<g.nedges; e++) if (g.sources[e]==v) {
                clause.push( E[e].getLit(true) );
            }
            sat.addClause(clause);
        }

        // For every ODD vertice, each outgoing edge must be activated
        for (int v=0; v<g.nvertices; v++) if (g.owners[v] == ODD) {
            for (int e=0; e<g.nedges; e++) if (g.sources[e]==v) {
                vec<Lit> clause;
                clause.push( V[v].getLit(false) );        
                clause.push( E[e].getLit(true) );
                sat.addClause(clause);
            }
        }

        // For every activated edge, the source vertex must be activated
        for (int v=0; v<g.nvertices; v++) {
            for (int e=0; e<g.nedges; e++) if (g.sources[e]==v) {
                vec<Lit> clause;
                clause.push( E[e].getLit(false) );
                clause.push( V[v].getLit(true) );
                sat.addClause(clause);
            }
        }

        // For every activated edge, the target vertex must be activated
        for (int w=0; w<g.nvertices; w++) if (w != g.start) {
            for (int e=0; e<g.nedges; e++) if (g.targets[e]==w) {
                vec<Lit> clause;
                clause.push( E[e].getLit(false) );
                clause.push( V[w].getLit(true) );
                sat.addClause(clause);
            }
        }

        // Every infinite ODD play must be avoided.
        new OddCycleFilter(g,V,E,filtertype);


        // Every unreachable vertex must be avoided.
        if (reachability == 1) {
            vec<vec<int>> _in, _out, _en;
            for (int v=0; v<g.owners.size(); v++) {
                _in.push();
                _out.push();
                for (int e=0; e<g.targets.size(); e++) {
                    if (g.targets[e]==v) {
                        _in[v].push(e);
                    }
                    if (g.sources[e]==v) {
                        _out[v].push(e);
                    }
                }
            }
            for (int e=0; e<g.targets.size(); e++) {
                _en.push();
                _en[e].push(g.sources[e]);
                _en[e].push(g.targets[e]);
            }
            new DReachabilityPropagator(g.start, V, E, _in, _out, _en);
        }

        //------------------------------------------------------------

        vec<Branching*> bv(static_cast<unsigned int>(g.nvertices));
        vec<Branching*> be(static_cast<unsigned int>(g.nedges));
        for (int i = g.nvertices; (i--) != 0;) bv[i] = &V[i];
        for (int i = g.nedges;    (i--) != 0;) be[i] = &E[i];
        
        branch(bv, VAR_INORDER, VAL_MIN);
        branch(be, VAR_INORDER, VAL_MIN);
        output_vars(bv);
        output_vars(be);
    }

    //----------------------------------------------------------------

    void fixVertices(std::initializer_list<int> vs,std::initializer_list<int> nvs={}) {
        for (int v : vs) {
            vec<Lit> clause;
            clause.push(V[v].getLit(true));
            sat.addClause(clause);
        }
        for (int v : nvs) {
            vec<Lit> clause;
            clause.push(V[v].getLit(false));
            sat.addClause(clause);
        }
    }

    //----------------------------------------------------------------

    void fixEdges(std::initializer_list<int> es,std::initializer_list<int> nes={}) {
        for (int e : es) {
            vec<Lit> clause;
            clause.push(E[e].getLit(true));
            sat.addClause(clause);
        }
        for (int e : nes) {
            vec<Lit> clause;
            clause.push(E[e].getLit(false));
            sat.addClause(clause);
        }
    }

    //----------------------------------------------------------------

    void fixStartingZero() {
        for (int i=0; i<g.sources.size(); i++) {
            g.sources[i]--;
            g.targets[i]--;
        }
        // start--;
    }

    //----------------------------------------------------------------

    void print(std::ostream& out)   override {
        switch (printtype) {
            case 1:
                out << "SAT ";
                break;
            case 2:
                out << "V=[";
                bool first = true;
                for (int i=0; i<V.size(); i++) {
                    if (V[i].isTrue()) {
                        if (first) first=false; else out << ",";
                        out << i;
                    }
                }
                out << "]\nE=[";
                first = true;
                for (int i=0; i<E.size(); i++) {
                    if (E[i].isTrue()) {
                        if (first) first=false; else out << ",";
                        out << i;
                    }
                }
                out << "]";
                break;
        }
    }
};

//----------------------------------------------------------------------