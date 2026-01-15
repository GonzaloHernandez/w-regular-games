#include "iostream"
#include "chuffed/vars/modelling.h"
#include "chuffed/core/propagator.h"
#include "initializer_list"

#include "prop_noc.cpp"

//===========================================================================

class ParityCondition : public WinningCondition {
    using WinningCondition::WinningCondition;
public:

    bool satisfy(int index,vec<int>& pathV,vec<int>& pathE) override {
        int m = g.colors[pathV[index]];
        for (int i=index+1; i<pathV.size(); i++) {
            if (g.compareColors(g.colors[pathV[i]],m,BET)) {
                m = g.colors[pathV[i]];
            }
        }
        return m%2==playerSAT;
    };
};

//===========================================================================

class EnergyCondition : public WinningCondition {
    using WinningCondition::WinningCondition;
public:

    bool satisfy(int index,vec<int>& pathV,vec<int>& pathE) override {
        int sum = 0;
        for (int i=index; i<pathE.size(); i++) {
            sum += g.weights[pathE[i]];
        }
        
        if (playerSAT == EVEN) {
            return sum >= 0;
        }
        return sum < 0;
    };
};

//===========================================================================

class MeanPayoffCondition : public WinningCondition {
    using WinningCondition::WinningCondition;
private:
    int threshold;
public:

    void setThreshold(int t) { threshold = t; }

    bool satisfy(int index,vec<int>& pathV,vec<int>& pathE) override {
        int sum = 0;
        for (int i=index; i<pathE.size(); i++) {
            sum += g.weights[pathE[i]];
        }
        int avg = sum / (pathE.size() - index);

        if (playerSAT == EVEN) {
            return avg >= threshold;
        }
        return avg < threshold;
    };
};

//===========================================================================

class NOCModel : public Problem {
private:
    Game& g;
    vec<BoolView> V;  
    vec<BoolView> E;
    int condition;
    int threshold;
    int printtype;
    parity_type playerSAT;
public:

    NOCModel(Game& g, int condition=0, int threshold=1, int printtype=0, 
        parity_type playerSAT=EVEN) 
    :g(g), condition(condition), threshold(threshold), printtype(printtype), 
        playerSAT(playerSAT)
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

        // --------------------------------------------------------------
        // For every PLAYER vertice, exactly one outgoing edge must be activated
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
                // -E_0 ∨ s_0
                vec<Lit> clause;
                clause.push(E[e].getLit(false));
                clause.push(s[0].getLit(true));
                sat.addClause(clause);
            }

            // Middle literals
            for (int i = 1; i < n - 1; i++) {
                int e = g.outs[v][i];

                // -s_{i-1} ∨ s_i
                {
                    vec<Lit> clause;
                    clause.push(s[i - 1].getLit(false));
                    clause.push(s[i].getLit(true));
                    sat.addClause(clause);
                }

                // -E_i ∨ ¬s_{i-1}
                {
                    vec<Lit> clause;
                    clause.push(E[e].getLit(false));
                    clause.push(s[i - 1].getLit(false));
                    sat.addClause(clause);
                }

                // -E_i ∨ s_i
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
                // -E_{n-1} ∨ -s_{n-2}
                vec<Lit> clause;
                clause.push(E[e_last].getLit(false));
                clause.push(s[n - 2].getLit(false));
                sat.addClause(clause);
            }
        }

        // --------------------------------------------------------------
        // For every OPPONENT vertice, each outgoing edge must be activated
        for (int v=0; v<g.nvertices; v++) if (g.owners[v] == opponent(playerSAT)) {
            for (int e : g.outs[v]) {
                vec<Lit> clause;
                clause.push( V[v].getLit(false) );        
                clause.push( E[e].getLit(true) );
                sat.addClause(clause);
            }
        }

        // --------------------------------------------------------------
        // For every activated edge, the target vertex must be activated
        for (int w=0; w<g.nvertices; w++) if (w != g.start) {
            for (int e : g.ins[w]) {
                vec<Lit> clause;
                clause.push( E[e].getLit(false) );
                clause.push( V[w].getLit(true) );
                sat.addClause(clause);
            }
        }

        // --------------------------------------------------------------
        // Every infinite OPPONENT play must be avoided regarding codition.
        WinningCondition* cond = nullptr;

        switch (condition) {
        case 0:
            cond = new ParityCondition(g,V,E,playerSAT);  break;
        case 1:
            cond = new EnergyCondition(g,V,E,playerSAT);  break;
        case 2:
            cond = new MeanPayoffCondition(g,V,E,playerSAT);
            ((MeanPayoffCondition*)cond)->setThreshold(threshold);
            break;
        default:
            cond = new ParityCondition(g,V,E,playerSAT);  break;
        }

        new NoOpponentCycle(g,V,E,playerSAT,*cond);

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