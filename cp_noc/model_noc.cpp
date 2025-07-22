#include "iostream"
#include "chuffed/vars/modelling.h"
#include "chuffed/core/propagator.h"
#include "initializer_list"

#include "prop_noc.cpp"
#include "checker_scc.cpp"

class NOCModel : public Problem {
public:
    static const int DZN    = 0;
    static const int GM     = 1;

private:
    Game& g;
    vec<BoolView> V;  
    vec<BoolView> E;
    int checker;
    int filter;
    int printtype;
    parity_type playerSAT;

public:

    NOCModel(Game& g, int checker, int filter, int printtype=0, parity_type playerSAT=EVEN) 
    : g(g), checker(checker), filter(filter), printtype(printtype), playerSAT(playerSAT)
    {
        V.growTo(g.nvertices);
        E.growTo(g.nedges);
        setupConstraints();
    }

    //----------------------------------------------------------------

    void setupConstraints() {

        for (int i=0; i<g.nvertices;  i++) V[i] = newBoolVar();
        for (int i=0; i<g.nedges;     i++) E[i] = newBoolVar();

        // Starting vertex
        fixVertices({g.start},{});

        // // For every EVEN vertice, at least one outgoing edge must be activated
        // for (int v=0; v<g.nvertices; v++) if (g.owners[v] == playerSAT) {
        //     vec<Lit> clause;
        //     clause.push( V[v].getLit(false) );
        //     // for (int e=0; e<g.nedges; e++) if (g.sources[e]==v) {
        //     for (auto& e : g.outs[v]) {
        //         clause.push( E[e].getLit(true) );
        //     }
        //     sat.addClause(clause);
        // }

        // // For every EVEN vertice, at most one outgoing edge must be activated
        // for (int v=0; v<g.nvertices; v++) if (g.owners[v] == playerSAT) {
        //     for (int i=0; i<g.outs[v].size()-1; i++) {
        //         for (int j=i+1; j<g.outs[v].size(); j++) {
        //             vec<Lit> clause;
        //             clause.push( E[g.outs[v][i]].getLit(false) );
        //             clause.push( E[g.outs[v][j]].getLit(false) );
        //             sat.addClause(clause);
        //         }
        //     }
        // }

        // --------------------------------------------------------------
        // For every EVEN vertice, exactly one outgoing edge must be activated
        for (int v=0; v<g.nvertices; v++) if (g.owners[v] == playerSAT) {

            int n = g.outs[v].size();

            // --- At least one ------------------------------------
            if (n == 0) continue;

            {
                vec<Lit> clause;
                clause.push( V[v].getLit(false) );
                for (int e : g.outs[v]) {
                    clause.push(E[e].getLit(true));
                }
                sat.addClause(clause); // E₀ ∨ E₁ ∨ ... ∨ Eₙ
            }

            // --- At most one -------------------------------------
            if (n == 1) continue;

            vec<BoolView> s(n - 1);
            for (int j = 0; j < n - 1; j++) s[j] = newBoolVar();

            // First literal
            {
                int e = g.outs[v][0];
                // ¬E₀ ∨ s₀
                vec<Lit> clause;
                clause.push(E[e].getLit(false));
                clause.push(s[0].getLit(true));
                sat.addClause(clause);
            }

            // Middle literals
            for (int i = 1; i < n - 1; i++) {
                int e = g.outs[v][i];

                // ¬s_{i-1} ∨ s_i
                {
                    vec<Lit> clause;
                    clause.push(s[i - 1].getLit(false));
                    clause.push(s[i].getLit(true));
                    sat.addClause(clause);
                }

                // ¬E_i ∨ ¬s_{i-1}
                {
                    vec<Lit> clause;
                    clause.push(E[e].getLit(false));
                    clause.push(s[i - 1].getLit(false));
                    sat.addClause(clause);
                }

                // ¬E_i ∨ s_i
                {
                    vec<Lit> clause;
                    clause.push(E[e].getLit(false));
                    clause.push(s[i].getLit(true));
                    sat.addClause(clause);
                }
            }

            // Last literal
            {
                int e_last = g.outs[v][n - 1];
                // ¬E_{n-1} ∨ ¬s_{n-2}
                vec<Lit> clause;
                clause.push(E[e_last].getLit(false));
                clause.push(s[n - 2].getLit(false));
                sat.addClause(clause);
            }
        }
        // --------------------------------------------------------------

        // For every ODD vertice, each outgoing edge must be activated
        for (int v=0; v<g.nvertices; v++) if (g.owners[v] == opponent(playerSAT)) {
            // for (int e=0; e<g.nedges; e++) if (g.sources[e]==v) {
            for (int e : g.outs[v]) {
                vec<Lit> clause;
                clause.push( V[v].getLit(false) );        
                clause.push( E[e].getLit(true) );
                sat.addClause(clause);
            }
        }

        // // For every activated edge, the source vertex must be activated
        // for (int v=0; v<g.nvertices; v++) {
        //     // for (int e=0; e<g.nedges; e++) if (g.sources[e]==v) {
        //     for (auto& e : g.outs[v]) {
        //         vec<Lit> clause;
        //         clause.push( E[e].getLit(false) );
        //         clause.push( V[v].getLit(true) );
        //         sat.addClause(clause);
        //     }
        // }

        // For every activated edge, the target vertex must be activated
        for (int w=0; w<g.nvertices; w++) if (w != g.start) {
            // for (int e=0; e<g.nedges; e++) if (g.targets[e]==w) {
            for (int e : g.ins[w]) {
                vec<Lit> clause;
                clause.push( E[e].getLit(false) );
                clause.push( V[w].getLit(true) );
                sat.addClause(clause);
            }
        }

        // // For every activated vertice, at least one incoming edge must be activated
        // for (int w=0; w<g.nvertices; w++) if (w != g.start ) {
        //     vec<Lit> clause;
        //     clause.push( V[w].getLit(false) );
        //     // for (int e=0; e<g.nedges; e++) if (g.targets[e]==w) {
        //     for (auto& e : g.ins[w]) {
        //         clause.push( E[e].getLit(true) );
        //     }
        //     sat.addClause(clause);
        // }

        // Every infinite ODD or EVEN play must be avoided.
        if (checker==1) new CheckerSCC(g,V,E,playerSAT);
        if (filter >=0) new NoOpponentCycle(g,E,filter,playerSAT);

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

    void print(std::ostream& out) override {
        if (printtype) {
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
    }
};

//----------------------------------------------------------------------