#include "iostream"
#include "chuffed/vars/modelling.h"
#include "chuffed/core/propagator.h"
#include "initializer_list"

#ifndef WINNING_CONDITIONS_H
#include "winning_conditions.h"
#endif

namespace Chuffed {

//=============================================================================

class CrossNoOpponentCycle : public Propagator {
private:
    Game& g;
    vec<BoolView> V;
    vec<BoolView> E;
    parity_type p;
    vec<WinningCondition*> conditions;
    bool    local;
    vec<int> assignment;

    const int   CF_DONE     = 1;
    const int   CF_CONFLICT = 2;
    const int   CF_STAY     = 3;
public:
    //-------------------------------------------------------------------------
    CrossNoOpponentCycle(Game& g, bool local, vec<BoolView>& V, vec<BoolView>& E, 
        parity_type p, vec<WinningCondition*> conditions)
    : g(g), local(local), V(V), E(E), p(p), conditions(conditions),
        assignment(g.nvertices,-1)
    {
        for (int i=0; i<g.nvertices;i++) V[i].attach(this, 1 , EVENT_F );
        for (int i=0; i<g.nedges;   i++) E[i].attach(this, 1 , EVENT_F );
    }
    //-------------------------------------------------------------------------
    int findVertex(int vertex,vec<int>& path) {
        for (int i=0; i<path.size(); i++) {
            if (path[i] == vertex) return i;
        }
        return -1;
    }
    //-------------------------------------------------------------------------
    void clausify(vec<int>& path, vec<BoolView> &B, vec<Lit>& lits,int from) {
        for (int i=from; i<path.size()-1; i++) {
            lits.push(B[path[i]].getValLit());
        }
    }
    //-------------------------------------------------------------------------
    bool satisfiedConditions(vec<int>& pathV,vec<int>& pathE,int index) {
        if (p==EVEN) {
            for (int i=0; i<conditions.size(); i++) {
                if (!conditions[i]->satisfy(pathV,pathE,index)) {
                    return false;
                }
            }
            return true;
        }
        else {
            for (int i=0; i<conditions.size(); i++) {
                if (conditions[i]->satisfy(pathV,pathE,index)) {
                    return true;
                }
            }
            return false;
        }
    }
    //-------------------------------------------------------------------------
    int filterEager(vec<int>& pathV, vec<int>& pathE, int v, 
        int lastEdge, bool definedEdge) 
    {
        int index = findVertex(v,pathV);
        if (index >= 0) {
            if (!satisfiedConditions(pathV,pathE,index)) {
                vec<Lit> lits;
                lits.push();
                clausify(pathE,E,lits,0);
                Clause* reason = Reason_new(lits);
                if (! E[lastEdge].setVal(p==EVEN?false:true,reason)) {
                    return CF_CONFLICT;
                }
            }
        }
        else if (definedEdge) {
            pathV.push(v);
            for (int e : g.outs[v]) {
                if (p==EVEN && E[e].isFalse())  continue;
                if (p==ODD && E[e].isTrue())    continue;

                int w = g.targets[e];
                pathE.push(e);
                int status = filterEager(pathV, pathE, w, e, p==EVEN?E[e].isTrue():E[e].isFalse());
                pathE.pop();
                if (status == CF_CONFLICT) {
                    return status;
                }
            }
            pathV.pop();
        }
        return CF_DONE;
    }
    //-------------------------------------------------------------------------
    bool propagate() override {
        vec<int> pathV;
        vec<int> pathE;

        if (local) {
            if (filterEager(pathV,pathE,g.init,-1,true) == CF_CONFLICT)
                return false;
        } else {
            for (int v=0; v<g.nvertices; v++) {
                if (V[v].isFixed()) {
                    if (filterEager(pathV,pathE,v,-1,true) == CF_CONFLICT)
                        return false;
                }
            }
        }
        return true;
    }
    //-------------------------------------------------------------------------
    void wakeup(int i, int) override {
        // if (done) return;
        pushInQueue();
    }
    //-------------------------------------------------------------------------
    void clearPropState() override {
        in_queue = false;
    }
};


//=============================================================================

class NOCBrancher : public Branching {
private:
    Game& g;
    vec<BoolView> V;
public:
    NOCBrancher(Game& g, vec<BoolView>& V) : g(g), V(V) {}
    bool finished() override {
        for (int i=0; i<V.size(); i++) {
            if (!V[i].isFixed()) return false;
        }
        return true;
    }
    double getScore(VarBranch vb) override {
        return 0;
    }
    DecInfo* branch() override {
        if (!V[g.init].isFixed()) {
            return V[g.init].branch();
        }
        for (int i=0; i<V.size(); i++) {
            if (!V[i].isFixed()) {
                return V[i].branch();
            }
        }
        return nullptr;
    }
};

//=============================================================================

class CrossNOCModel : public Problem {
private:
    Game& g;
    vec<BoolView>       V;
    vec<vec<BoolView>>  E;
    std::vector<bool>   conditions;
    float threshold;
    int printtype;
    vec<vec<int>>&      sol;
    bool                local;
public:

    CrossNOCModel(Game& g, bool local, vec<vec<int>>& sol,std::vector<bool> conditions, 
        float threshold=1.0f, int printtype=0) 
    :g(g), local(local), sol(sol), conditions(conditions), threshold(threshold), 
        printtype(printtype)
    {
        V.growTo(g.nvertices);
        E.growTo(2);
        E[EVEN].growTo(g.nedges);
        E[ODD ].growTo(g.nedges);
        for (int i=0; i<g.nvertices;  i++) 
            V[i] = newBoolVar();
        for (int i=0; i<g.nedges; i++) {
            E[EVEN][i] = newBoolVar();
            E[ODD ][i] = newBoolVar();
        }
        setupConstraints(EVEN);
        setupConstraints(ODD);
        //---------------------------------------------------------------------

        vec<Branching*> bv(static_cast<unsigned int>(g.nvertices));
        vec<Branching*> be0(static_cast<unsigned int>(g.nedges));
        vec<Branching*> be1(static_cast<unsigned int>(g.nedges));
        for (int i = g.nvertices; (i--) != 0;) bv[i] = &V[i];
        for (int i = g.nedges; (i--) != 0;) be0[i] = &E[EVEN][i];
        for (int i = g.nedges; (i--) != 0;) be1[i] = &E[ODD][i];
        
        // branch(bv, VAR_INORDER, VAL_MIN);
        engine.branching->add(new NOCBrancher(g,V));
        branch(be0, VAR_INORDER, VAL_MIN);
        branch(be1, VAR_INORDER, VAL_MIN);
        output_vars(bv);
        output_vars(be0);
        output_vars(be1);
    }

    //-------------------------------------------------------------------------

    void setupConstraints(parity_type p) {

        // --------------------------------------------------------------------
        // For every active PLAYER vertex, one outgoing edge must be activated
        for (int v=0; v<g.nvertices; v++) if (g.owners[v] == p) {

            int n = g.outs[v].size();

            // --- At least one -----------------------------------------------
            if (n == 0) continue;

            {
                vec<Lit> clause;
                clause.push( V[v].getLit(p==EVEN?false:true) );
                for (int e : g.outs[v]) {
                    clause.push(E[p][e].getLit(p==EVEN?true:false));
                }
                sat.addClause(clause); // E_0 \/ E_1 \/ ... \/ E_n
            }

            // --- At most one ------------------------------------------------
            if (n == 1) continue;

            vec<BoolView> s(n - 1);
            for (int j = 0; j < n - 1; j++) s[j] = newBoolVar();

            // First literal
            {
                int e = g.outs[v][0];
                // -E_0 \/ s_0
                vec<Lit> clause;
                clause.push(E[p][e].getLit(p==EVEN?false:true));
                clause.push(s[0].getLit(true));
                sat.addClause(clause);
            }

            // Middle literals
            for (int i = 1; i < n - 1; i++) {
                int e = g.outs[v][i];

                // -s_{i-1} \/ s_i
                {
                    vec<Lit> clause;
                    clause.push(s[i - 1].getLit(false));
                    clause.push(s[i].getLit(true));
                    sat.addClause(clause);
                }

                // -E_i \/ -s_{i-1}
                {
                    vec<Lit> clause;
                    clause.push(E[p][e].getLit(p==EVEN?false:true));
                    clause.push(s[i - 1].getLit(false));
                    sat.addClause(clause);
                }

                // -E_i \/ s_i
                {
                    vec<Lit> clause;
                    clause.push(E[p][e].getLit(p==EVEN?false:true));
                    clause.push(s[i].getLit(true));
                    sat.addClause(clause);
                }
            }

            // Last literal
            {
                int e_last = g.outs[v][n - 1];
                // -E_{n-1} \/ -s_{n-2}
                vec<Lit> clause;
                clause.push(E[p][e_last].getLit(p==EVEN?false:true));
                clause.push(s[n - 2].getLit(false));
                sat.addClause(clause);
            }
        }

        // --------------------------------------------------------------------
        // For every active/inactive OPPONENT vertice, each outgoing edge must 
        // be activated/deactivated
        for (int v=0; v<g.nvertices; v++) if (g.owners[v]==opponent(p)) {
            for (int e : g.outs[v]) {
                vec<Lit> clause;
                clause.push( V[v].getLit(p==EVEN?false:true) );        
                clause.push( E[p][e].getLit(p==EVEN?true:false) );
                sat.addClause(clause);
            }
        }

        // --------------------------------------------------------------------
        // For every active/inactive edge, the target vertex must be activated/
        // deactivated
        for (int e=0; e<g.nedges; e++) {
            int w = g.targets[e];
            vec<Lit> clause;
            clause.push( E[p][e].getLit(p==EVEN?false:true) );
            clause.push( V[w].getLit(p==EVEN?true:false) );
            sat.addClause(clause);
        }

        // --------------------------------------------------------------------
        // Every infinite OPPONENT play must be avoided regarding codition.
        WinningCondition* cond = nullptr;

        vec<WinningCondition*> conds;

        if (conditions[0]) {
            ParityCondition* c = new ParityCondition(g,p);
            conds.push(c);
        }
            
        if (conditions[1]) {
            EnergyCondition* c = new EnergyCondition(g,p);
            conds.push(c);
        }
            
        if (conditions[2]) {
            MeanPayoffCondition* c = new MeanPayoffCondition(g,p);
            c->setThreshold(threshold);
            conds.push(c);
        }

        new CrossNoOpponentCycle(g,local,V,E[p],p,conds);
    }

    //-------------------------------------------------------------------------

    void fixVertices(   std::initializer_list<int> vs,
                        std::initializer_list<int> nvs={})
    {
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

    // //-------------------------------------------------------------------------

    // void fixEdges(  std::initializer_list<int> es,
    //                 std::initializer_list<int> nes={}) 
    // {
    //     for (int e : es) {
    //         vec<Lit> clause;
    //         clause.push(E[e].getLit(true));
    //         sat.addClause(clause);
    //     }
    //     for (int e : nes) {
    //         vec<Lit> clause;
    //         clause.push(E[e].getLit(false));
    //         sat.addClause(clause);
    //     }
    // }

    //-------------------------------------------------------------------------

    void print(std::ostream& out) override {
        for (int i=0; i<g.nvertices; i++) {
            if (V[i].isTrue()) {
                sol[EVEN].push(i);
            } else {
                sol[ODD].push(i);
            }
        }

        if (printtype==2) {
            out << "EVENs =[";
            bool first = true;
            for (int i=0; i<V.size(); i++) {
                if (V[i].isTrue()) {
                    if (first) first=false; else out << ",";
                    out << i;
                }
            }
            out << "]\nODDs  =[";
            first = true;
            for (int i=0; i<V.size(); i++) {
                if (V[i].isFalse()) {
                    if (first) first=false; else out << ",";
                    out << i;
                }
            }
            // out << "]\n-------------\n";
            // for (int v=0; v<g.nvertices; v++) {
            //     out << v << ":\t";
            //     for (int a=0; a<assignment[v].size(); a++) {
            //         if (a>0) out << " ";
            //         out << (assignment[v][a]==-1?"\033[0;31m?\033[0m":(assignment[v][a]==0?"-":"+"));
            //     }
            //     out << "\n";
            // }
            // out << "]\nEodd =[";
            // for (int i=0; i<E[ODD].size(); i++) {
            //     out << (i>0?",":"") << E[ODD][i].getVal();
            // }
            out << "]\n";
        }
        if (printtype>=1) {
            if (V[g.init].isTrue()) {
                out << g.init << ":EVEN";
            } else {
                out << g.init << ":ODD";
            }
        }
    }
};

} // namespace Chuffed