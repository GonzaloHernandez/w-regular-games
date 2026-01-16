#ifndef GAME_H
#include "game.h"
#endif

#include "vec.h"
#include <gecode/int.hh>
#include <gecode/minimodel.hh>

// class NoOpponentCycle : public Gecode::Propagator {
// protected:
//     Game& g;
//     vec<Gecode::BoolVar> V;
//     vec<Gecode::BoolVar> E;
//     parity_type playerSAT;

// };

class NocModelGecode : public Gecode::Space {
protected:
    Game& g;
    Gecode::BoolVarArray V;
    Gecode::BoolVarArray E;
    std::vector<bool> conditions;
    int threshold;
    parity_type playerSAT;
public:
// --------------------------------------------------------------
    NocModelGecode(Game& g, std::vector<bool> conditions, int threshold=1, 
        parity_type playerSAT=EVEN) 
    : V(*this, g.nvertices, 0, 1),E(*this, g.nedges, 0, 1),
        g(g), conditions(conditions), threshold(threshold), playerSAT(playerSAT)
    {
        setupConstraints();
        Gecode::branch(*this, V, Gecode::BOOL_VAR_NONE(), Gecode::BOOL_VAL_MIN());
        Gecode::branch(*this, E, Gecode::BOOL_VAR_NONE(), Gecode::BOOL_VAL_MIN());
    }

    // --------------------------------------------------------------
    void setupConstraints() {
        // Starting vertex
        Gecode::rel(*this, V[g.start], Gecode::IRT_EQ, 1);

        // --------------------------------------------------------------
        // For every PLAYER active vertex, exactly one outgoing edge must be activated
        for (int v=0; v<g.nvertices; v++) if (g.owners[v] == playerSAT) {
            int n = g.outs[v].size();
            Gecode::BoolVarArgs edgeVars(n);
            for (int i = 0; i < n; i++) {
                int e = g.outs[v][i];
                edgeVars[i] = E[e];
            }
            Gecode::IntVar targetSum(*this, 0, 1);
            Gecode::channel(*this, V[v], targetSum);
            Gecode::linear(*this, edgeVars, Gecode::IRT_EQ, targetSum);
        }

        // --------------------------------------------------------------
        // For every OPPONENT active vertex, each outgoing edge must be activated
        for (int v=0; v<g.nvertices; v++) if (g.owners[v] == opponent(playerSAT)) {
            int n = g.outs[v].size();
            Gecode::BoolVarArgs edgeVars(n);
            for (int i = 0; i < n; i++) {
                int e = g.outs[v][i];
                edgeVars[i] = E[e];
                Gecode::rel(*this, V[v], Gecode::BOT_IMP, E[e], 1);
            }
            // Gecode::rel(*this, edgeVars, Gecode::IRT_EQ, 1);
        }

        // --------------------------------------------------------------
        // For every activated edge, the target vertex must be activated
        for (int w = 0; w < g.nvertices; w++) {
            if (w != g.start) {
                for (int e : g.ins[w]) {
                    Gecode::rel(*this, E[e], Gecode::BOT_IMP, V[w], 1);
                }
            }
        }
    }

    // --------------------------------------------------------------
    NocModelGecode(NocModelGecode& source) 
    : Gecode::Space(source), g(source.g), conditions(source.conditions), 
        threshold(source.threshold), playerSAT(source.playerSAT) 
    {
        V.update(*this, source.V);
        E.update(*this, source.E);
    }

    // --------------------------------------------------------------
    virtual Gecode::Space* copy(void) override {
        return new NocModelGecode(*this);
    }

    void print() const {
        std::cout << "{";
        bool first = true;
        for (int v=0; v<g.nvertices; v++) {
            if (V[v].val() == 0) continue;
            if (!first) std::cout << ",";
            std::cout << v;
            first = false;
        }
        std::cout << "} {";
        first = true;
        for (int e=0; e<g.nedges; e++) {
            if (E[e].val() == 0) continue;
            if (!first) std::cout << ",";
            std::cout << e;
            first = false;
        }
        std::cout << "}\n";
    }

};