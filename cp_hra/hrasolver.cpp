#ifndef CPP_GAME
#include "../chuffed-patch/game.cpp"
#endif

#include "iostream"
#include "chuffed/vars/modelling.h"
#include "chuffed/core/propagator.h"

class HRAModel : public Problem {
private:
    Game&   g;
    vec<BoolView>   V;
    vec<BoolView>   E;

public:
    HRAModel(Game&g) : g(g) {
        V.growTo(g.nvertices);
        E.growTo(g.nedges);

        for (int i=0; i<g.nvertices;  i++) V[i] = newBoolVar();
        for (int i=0; i<g.nedges;     i++) E[i] = newBoolVar();

        vec<Lit> clause;
        clause.push(V[g.start].getLit(true));
        sat.addClause(clause);

        // For every vertice, at least one outgoing edge must be activated
        for (int v=0; v<g.nvertices; v++) {
            vec<Lit> clause;
            for (int e=0; e<g.nedges; e++) if (g.sources[e]==v) {
                clause.push( E[e].getLit(true) );
            }
            sat.addClause(clause);
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

    void print(std::ostream& out)   override {
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
    }
};