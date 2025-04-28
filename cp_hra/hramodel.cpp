#ifndef game_h
#include "../chuffed-patch/game.h"
#endif

#include "iostream"
#include "chuffed/vars/modelling.h"
#include "chuffed/core/propagator.h"

#include "chuffed/globals/dconnected.h"
#include "../chuffed-patch/qualify.cpp"

#ifndef debugchuffed_h
#include "debugchuffed.h"
#endif

#ifndef debugstd_h
#include "debugstd.h"
#endif

class HRAModel : public Problem {
private:
    Game&   g;
    vec<BoolView>   V;
    vec<BoolView>   E;
    vec<BoolView>   Q;
    vec<BoolView>   C;

public:

    HRAModel(Game&g) : g(g) {
        V.growTo(g.nvertices);
        E.growTo(g.nedges);
        Q.growTo(g.nvertices);
        C.growTo(g.nvertices);

        for (int i=0; i<g.nvertices;  i++) V[i] = newBoolVar();
        for (int i=0; i<g.nedges;     i++) E[i] = newBoolVar();
        for (int i=0; i<g.nvertices;  i++) Q[i] = newBoolVar();
        for (int i=0; i<g.nvertices;  i++) C[i] = newBoolVar();

        // -------------------------------------------------------------------
        // Connecting the graph

        // Activating first vertex
        vec<Lit> clause;
        clause.push(V[g.start].getLit(true));
        sat.addClause(clause);

        // For every vertice needs to have one outgoing edge activated
        // for (int v=0; v<g.nvertices; v++) {
        //     vec<Lit> clause;
        //     clause.push( V[v].getLit(false) );
        //     for (auto& e : g.vedges[v]) {
        //         clause.push( E[e].getLit(true) );
        //     }
        //     sat.addClause(clause);
        // }

        // For every vertice needs to every outgoing edge activated
        // \bigwedge v \in V ( \mathcal{V}_v \rightarrow  \exists e \in E \mathcal{E}_e)
        for (int v=0; v<g.nvertices; v++) {
            for (auto& e : g.vedges[v]) {
                vec<Lit> clause;
                clause.push( V[v].getLit(false) );
                clause.push( E[e].getLit(true) );
                sat.addClause(clause);
            }
        }

        // For every activated edge, the target vertex must be activated
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
        // new DReachabilityPropagator(g.start, V, E, _in, _out, _en);

        // for every edge, the source vertex must be activated
        for (int e=0; e<g.nedges; e++) {
            vec<Lit> clause;
            clause.push( E[e].getLit(false) );
            clause.push( V[g.sources[e]].getLit(true) );
            sat.addClause(clause);
        }

        // -------------------------------------------------------------------
        // Qualifying vertices on the type of play that can force

        // If v is EVEN and exists one EVEN edge or Even cycle, from v the play is EVEN
        for (int v=0; v<g.nvertices; v++) {
            for (auto& e : g.vedges[v]) { int w = g.targets[e];
                vec<Lit> clause;
                clause.push( g.owners[v]==EVEN?lit_False:lit_True );
                clause.push( Q[w].getLit(!EVEN) );
                clause.push( Q[v].getLit(EVEN) );
                sat.addClause(clause);
            }
            vec<Lit> clause;
            clause.push( g.owners[v]==EVEN?lit_False:lit_True );
            clause.push( C[v].getLit(!EVEN) );
            clause.push( Q[v].getLit(EVEN) );
            sat.addClause(clause);
        }

        // If v is ODD and exists one ODD edge, from v the play is ODD
        for (int v=0; v<g.nvertices; v++) {
            for (auto& e : g.vedges[v]) { int w = g.targets[e];
                vec<Lit> clause;
                clause.push( g.owners[v]==ODD?lit_False:lit_True );
                clause.push( Q[w].getLit(!ODD) );
                clause.push( Q[v].getLit(ODD) );
                sat.addClause(clause);
            }
            vec<Lit> clause;
            clause.push( g.owners[v]==ODD?lit_False:lit_True );
            clause.push( C[v].getLit(!ODD) );
            clause.push( Q[v].getLit(ODD) );
            sat.addClause(clause);
        }

        // If v is EVEN and all edges are ODD, from v the play is ODD
        for (int v=0; v<g.nvertices; v++) {
            vec<Lit> clause;
            clause.push( g.owners[v]==EVEN?lit_False:lit_True );
            for (auto& e : g.vedges[v]) { int w = g.targets[e];
                clause.push( Q[w].getLit(!ODD) );
            }
            clause.push( C[v].getLit(!ODD) );
            clause.push( Q[v].getLit(ODD) );
            sat.addClause(clause);
        }

        // If v is ODD and all edges are EVEN, from v the play is EVEN
        for (int v=0; v<g.nvertices; v++) {
            vec<Lit> clause;
            clause.push( g.owners[v]==ODD?lit_False:lit_True );
            for (auto& e : g.vedges[v]) { int w = g.targets[e];
                clause.push( Q[w].getLit(!EVEN) );
            }
            clause.push( C[v].getLit(!EVEN) );
            clause.push( Q[v].getLit(EVEN) );
            sat.addClause(clause);
        }

        // for (int v=0; v<g.nvertices; v++) {
        //     {
        //         vec<Lit> clause;
        //         clause.push( V[v].getLit(true) );
        //         clause.push( Q[v].getLit(true) );
        //         clause.push( Q[g.start].getLit(false) );
        //         sat.addClause(clause);
        //     }
        //     {
        //         vec<Lit> clause;
        //         clause.push( V[v].getLit(true) );
        //         clause.push( Q[v].getLit(false) );
        //         clause.push( Q[g.start].getLit(true) );
        //         sat.addClause(clause);    
        //     }
        // }

        // -------------------------------------------------------------------

        new Qualify(g,V,E,Q,C);

        // -------------------------------------------------------------------

        // fix(Q,{},{0,1,2,3,4});

        // -------------------------------------------------------------------

        vec<Branching*> bv(static_cast<unsigned int>(g.nvertices));
        vec<Branching*> be(static_cast<unsigned int>(g.nedges));
        vec<Branching*> bq(static_cast<unsigned int>(g.nvertices));
        vec<Branching*> bc(static_cast<unsigned int>(g.nvertices));
        for (int i = g.nvertices; (i--) != 0;) bv[i] = &V[i];
        for (int i = g.nedges;    (i--) != 0;) be[i] = &E[i];
        for (int i = g.nvertices; (i--) != 0;) bq[i] = &Q[i];
        for (int i = g.nvertices; (i--) != 0;) bc[i] = &C[i];
        
        branch(bv, VAR_INORDER, VAL_MIN);
        branch(be, VAR_INORDER, VAL_MIN);
        branch(bq, VAR_INORDER, VAL_MIN);
        branch(bc, VAR_INORDER, VAL_MIN);
        output_vars(bv);
        output_vars(be);
        output_vars(bq);
        output_vars(bc);
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
        out << "]\nQ=[";
        first = true;
        for (int i=0; i<Q.size(); i++) {
            out << (!i?"":",") << Q[i].getVal();
        }
        out << "]\nC=[";
        first = true;
        for (int i=0; i<C.size(); i++) {
            out << (!i?"":",") << C[i].getVal();
        }
        out << "]";
    }
};

int main(int argc, char *argv[])
{
    launchdebugchuffed();
    launchdebugstd();
    Game g(DZN, "data/game-other.dzn",0,MIN);

    HRAModel* model = new HRAModel(g);

    so.nof_solutions = 0;
    engine.solve(model);

    // delete model;
    return 0;
}
