#ifndef CPP_GAME
#include "../chuffed-patch/game.cpp"
#endif

#include "iostream"
#include "chuffed/vars/modelling.h"
#include "chuffed/core/propagator.h"

#include "chuffed/globals/dconnected.h"

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

        // Activating first vertex
        vec<Lit> clause;
        clause.push(V[g.start].getLit(true));
        sat.addClause(clause);

        // For every vertice needs to have one outgoing edge activated
        for (int v=0; v<g.nvertices; v++) {

            vec<Lit> clause;
            clause.push( V[v].getLit(false) );
            for (auto& e : g.vedges[v]) {
                clause.push( E[e].getLit(true) );
            }
            sat.addClause(clause);

            for (auto& e1 : g.vedges[v]) {
                for (auto& e2 : g.vedges[v]) if (e1 < e2) {
                    vec<Lit> clause;
                    clause.push( V[v].getLit(false) );
                    clause.push( E[e1].getLit(false) );
                    clause.push( E[e2].getLit(false) );
                    sat.addClause(clause);
                }
            }
        }

        // For every activated edge, the source and target vertex must be activated
        for (int e=0; e<g.nedges; e++) {
            vec<Lit> clause;
            clause.push( E[e].getLit(false) );
            clause.push( V[g.targets[e]].getLit(true) );
            sat.addClause(clause);
        }

        // For every vertice, one incoming edge must be activated
        for (int w=0; w<g.nvertices; w++) if (w != g.start) {
            vec<Lit> clause;
            clause.push( V[w].getLit(false) );
            for (int e=0; e<g.nedges; e++) if (g.targets[e] == w) {
                clause.push( E[e].getLit(true) );
            }
            sat.addClause(clause);
        }

        // Every unreachable vertex must be avoided.
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

    void fix(   vec<BoolView>&B, 
                std::initializer_list<int> bs,
                std::initializer_list<int> nbs={}) 
    {
        for (int b : bs) {
            vec<Lit> clause;
            clause.push(B[b].getLit(true));
            sat.addClause(clause);
        }
        for (int b : nbs) {
            vec<Lit> clause;
            clause.push(B[b].getLit(false));
            sat.addClause(clause);
        }
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