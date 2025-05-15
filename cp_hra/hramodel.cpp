#ifndef game_h
#include "../chuffed-patch/game.h"
#endif

#include "iostream"
#include "chuffed/vars/modelling.h"
#include "chuffed/core/propagator.h"

#include "chuffed/globals/dconnected.h"
#include "../chuffed-patch/qualifynodeviations.cpp"
// #include "../chuffed-patch/qualifywithdeviations.cpp"
#include "../chuffed-patch/coattractor.cpp"

#ifndef debugchuffed_h
#include "debugchuffed.h"
#endif

#ifndef debugstd_h
#include "debugstd.h"
#endif

class HRAModel : public Problem {
private:
    Game&   g;
    vec<BoolView>   Q;

public:

    HRAModel(Game&g) : g(g) {
        Q.growTo(g.nvertices);

        for (int i=0; i<g.nvertices; i++) Q[i] = newBoolVar();

        // -------------------------------------------------------------------
        // Qualifying vertices on the type of play that can force

        // v wins if some outgoing edge is the same color as v
        for (int v=0; v<g.nvertices; v++) {
            for (auto& e : g.outs[v]) { int w = g.targets[e];
                vec<Lit> clause;
                clause.push( Q[w].getLit( !g.owners[v] ) );
                clause.push( Q[v].getLit( g.owners[v] ) );
                sat.addClause(clause);
            }
        }

        // v losses if every outgoing edge is not the same color as v
        for (int v=0; v<g.nvertices; v++) {
            vec<Lit> clause;
            for (auto& e : g.outs[v]) { int w = g.targets[e];
                clause.push( Q[w].getLit( g.owners[v]) );
            }
            clause.push( Q[v].getLit( !g.owners[v]) );
            sat.addClause(clause);
        }

        // -------------------------------------------------------------------

        // new QualifyNoDeviations(g,Q);
        new CoAttractor(g,Q);
        // new QualifyWithDeviations(g,V,E,Q);

        // -------------------------------------------------------------------

        // fix(Q,{},{0,1,2,3,4});

        // -------------------------------------------------------------------

        vec<Branching*> bq(static_cast<unsigned int>(g.nvertices));
        for (int i = g.nvertices; (i--) != 0;) bq[i] = &Q[i];
        
        branch(bq, VAR_INORDER, VAL_MIN);
        output_vars(bq);
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
        bool first = true;
        out << "Q=[";
        first = true;
        for (int i=0; i<Q.size(); i++) {
            out << (!i?"":",") << Q[i].getVal();
        }
        out << "]";
    }

    //----------------------------------------------------------------

    bool getVal(int v) {
        return Q[v].getVal();
    }

};

// int main(int argc, char *argv[])
// {
//     launchdebugchuffed();
//     launchdebugstd();
//     Game g(DZN, "data/game-wikidzn",0,MAX);

//     HRAModel* model = new HRAModel(g);

//     so.nof_solutions = 0;
//     engine.solve(model);

//     // delete model;
//     return 0;
// }
