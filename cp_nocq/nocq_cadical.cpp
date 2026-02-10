#include "iostream"
#include "cadical.hpp"

#ifndef WINNING_CONDITIONS_H
#include "winning_conditions.h"
#endif

namespace CaDiCaL {

#define BoolView int
#define Lit int

//=============================================================================

class Different : public ExternalPropagator {
private:
    vec<int> reason;
    size_t index;
    vec<int> vals; 

public:
    Different() : vals(3, 0), index(0) {}
    //-------------------------------------------------------------------------
    void notify_assignment(const std::vector<int> &lits) override {
        for (int lit : lits) {
            int v = abs(lit);
            if (v < vals.size()) {
                vals[v] = (lit > 0) ? 1 : -1;
            }
        }
    }
    //-------------------------------------------------------------------------
    void notify_backtrack(size_t new_level) override {
        if (new_level == 0) {
            for(int v=1; v<vals.size(); v++) {
                vals[v] = 0;
            }
        }
    }
    //-------------------------------------------------------------------------
    int cb_propagate() override {
        if (vals[1] != 0 && vals[2] != 0) {
            if (vals[1] == vals[2]) {

                reason.clear();
                reason.push(vals[1] < 0 ? 1 : -1);
                reason.push(vals[2] < 0 ? 2 : -2);
                reason.push(0);
                
                index = 0; 
                return vals[1] < 0 ? 1 : -1;
            }
        }
        return 0;
    }
    //-------------------------------------------------------------------------
    int cb_add_reason_clause_lit(int propagated_lit) override {
        if (index < reason.size()) {
            int lit = reason[index++];
            return lit;
        }
        return 0;
    }
    //-------------------------------------------------------------------------
    int cb_add_external_clause_lit() override {
        return 0;
    }
    //-------------------------------------------------------------------------
    bool cb_has_external_clause(bool &is_forgettable) override {
        is_forgettable = false;
        return false;
    }
    //-------------------------------------------------------------------------
    void notify_new_decision_level () override {
    }
    //-------------------------------------------------------------------------
    bool cb_check_found_model (const std::vector<int> &model) override {
        if (index < reason.size()) {
            return true;
        }
        return false;
    }

};

//=============================================================================

class NoOpponentCycle : public ExternalPropagator {
private:
    Game& g;
    vec<BoolView>& V;
    vec<BoolView>& E;
    vec<int>    assigns;
    parity_type playerSAT;
    vec<WinningCondition*> conditions;

    const int   CF_DONE     = 1;
    const int   CF_CONFLICT = 2;
    const int   CF_STAY     = 3;

    vec<Lit> reason;
    size_t index;

    int getValLit(int v){ return assigns[v]*v; }
    bool isFixed(int v) { return assigns[v] != 0; }
    bool isFalse(int v) { return assigns[v] == -1; }
    bool isTrue(int v)  { return assigns[v] == 1; }

public:
    NoOpponentCycle(Game& game, vec<BoolView>& V, vec<BoolView>& E, 
        parity_type playerSAT, vec<WinningCondition*> conditions)
    : g(game), V(V), E(E), playerSAT(playerSAT), conditions(conditions),
        index(0), assigns(g.nvertices+g.nedges+1,0)
    {
    }
    //-------------------------------------------------------------------------
    void notify_assignment(const std::vector<int> &lits) override {
        for (int lit : lits) {
            int v = abs(lit);
            assigns[v] = (lit > 0) ? 1 : -1;
        }
    }
    //-------------------------------------------------------------------------
    void notify_backtrack(size_t new_level) override {
        if (new_level == 0) {
            for(int v=1; v<assigns.size(); v++) {
                assigns[v] = 0;
            }
        }
    }
    //-------------------------------------------------------------------------
    int cb_propagate() override {
        vec<int> pathV;
        vec<int> pathE;

        if (filterEager(pathV,pathE,g.init,-1,true) == CF_CONFLICT)
            return reason[0];

        return 0;
    }
    //-------------------------------------------------------------------------
    int cb_add_reason_clause_lit(int propagated_lit) override {
        if (index < reason.size()) {
            int lit = reason[index++];
            return lit;
        }
        return 0;
    }
    //-------------------------------------------------------------------------
    int cb_add_external_clause_lit() override {
        return 0;
    }
    //-------------------------------------------------------------------------
    bool cb_has_external_clause(bool &is_forgettable) override {
        is_forgettable = false;
        return false;
    }
    //-------------------------------------------------------------------------
    void notify_new_decision_level () override {
    }
    //-------------------------------------------------------------------------
    bool cb_check_found_model (const std::vector<int> &model) override {
        if (index < reason.size()) {
            return true;
        }
        return false;
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
            lits.push( getValLit(B[path[i]]) );
        }
    }
    //-------------------------------------------------------------------------
    bool satisfiedConditions(vec<int>& pathV,vec<int>& pathE,int index) {
        if (playerSAT==EVEN) {
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
                reason.clear();
                reason.push(-E[lastEdge]);
                clausify(pathE,E,reason,0);
                return CF_CONFLICT;
            }
        }
        else if (definedEdge) {
            pathV.push(v);
            for (int e : g.outs[v]) {
                if (isFalse(E[e])) continue;

                int w = g.targets[e];
                pathE.push(e);
                int status = filterEager(pathV, pathE, w, e, isTrue(E[e]));
                pathE.pop();
                if (status == CF_CONFLICT) {
                    return status;
                }
            }
            pathV.pop();
        }
        return CF_DONE;
    }
};

//=============================================================================

class NOCQModel {
private:
    Game& g;
    vec<BoolView> V;
    vec<BoolView> E;
    std::vector<bool> conditions;
    int threshold;
    parity_type playerSAT;

    int pool=0;
    Solver* solver;

    int newBoolVar ()           { pool += 1;    return pool; }
    int newBoolVars(int size)   { pool += size; return pool-size+1; }
public:
    //-------------------------------------------------------------------------
    NOCQModel(Game& game, std::vector<bool> conditions, int threshold=1, 
        parity_type playerSAT=EVEN)
    : g(game), conditions(conditions), threshold(threshold), playerSAT(playerSAT) {
        solver = new Solver();
        solver->set("factor",0);

        V.growTo(g.nvertices,0);
        E.growTo(g.nedges,0);

        setupConstraints();

        for (int i=0; i<g.nvertices; i++) {
            solver->add_observed_var(V[i]);
        }
        for (int i=0; i<g.nedges; i++) {
            solver->add_observed_var(E[i]);
        }
    }
    //-------------------------------------------------------------------------
    void setupConstraints() {
        for (int i=0; i<g.nvertices;  i++) V[i] = newBoolVar();
        for (int i=0; i<g.nedges;     i++) E[i] = newBoolVar();

        solver->clause(V[g.init]);
        // --------------------------------------------------------------------
        // For every active PLAYER vertex, one outgoing edge must be activated
        for (int v=0; v<g.nvertices; v++) if (g.owners[v] == playerSAT) {

            int n = g.outs[v].size();

            // --- At least one -----------------------------------------------
            if (n == 0) continue;

            {
                solver->add( -V[v] );
                for (int e : g.outs[v]) {
                    solver->add( E[e] );
                }
                solver->add(0); // E_0 \/ E_1 \/ ... \/ E_n
            }
            // --- At most one ------------------------------------------------
            if (n == 1) continue;

            vec<BoolView> s(n - 1);
            for (int j = 0; j < n - 1; j++) s[j] = newBoolVar();

            // First literal
            {
                int e = g.outs[v][0];
                // -E_0 \/ s_0
                solver->clause( -E[e], s[0] );
            }

            // Middle literals
            for (int i = 1; i < n - 1; i++) {
                int e = g.outs[v][i];

                // -s_{i-1} \/ s_i
                {
                    solver->clause( -s[i-1], s[i] );
                }

                // -E_i \/ -s_{i-1}
                {
                    solver->clause( -E[e], -s[i-1] );
                }

                // -E_i \/ s_i
                {
                    solver->clause( -E[e], s[i] );
                }
            }

            // Last literal
            {
                int e_last = g.outs[v][n - 1];
                // -E_{n-1} \/ -s_{n-2}
                solver->clause( -E[e_last], -s[n - 2] );
            }
        }
    
        // --------------------------------------------------------------------
        // For every active OPPONENT vertice, each outgoing edge must be activated
        for (int v=0; v<g.nvertices; v++) if (g.owners[v]==opponent(playerSAT)) {
            for (int e : g.outs[v]) {
                solver->clause( -V[v], E[e] );
            }
        }

        // --------------------------------------------------------------------
        // For every active edge, the target vertex must be activated
        for (int w=0; w<g.nvertices; w++) if (w != g.init) {
            for (int e : g.ins[w]) {
                solver->clause( -E[e], V[w] );
            }
        }

        // --------------------------------------------------------------------
        // Every infinite OPPONENT play must be avoided regarding codition.
        WinningCondition* cond = nullptr;

        vec<WinningCondition*> conds;

        if (conditions[0]) {
            ParityCondition* c = new ParityCondition(g,playerSAT);
            conds.push(c);
        }
            
        if (conditions[1]) {
            EnergyCondition* c = new EnergyCondition(g,playerSAT);
            conds.push(c);
        }
            
        if (conditions[2]) {
            MeanPayoffCondition* c = new MeanPayoffCondition(g,playerSAT);
            c->setThreshold(threshold);
            conds.push(c);
        }

        NoOpponentCycle *noc = new NoOpponentCycle(g,V,E,playerSAT,conds);
        solver->connect_external_propagator(noc);

    }
    //-------------------------------------------------------------------------
    ~NOCQModel() {
        delete solver;
    }
    //-------------------------------------------------------------------------
    bool solve() {
        int res = solver->solve();

        if (res == 10) {
            return true;
        } else {
            return false;
        }
    }
    //-------------------------------------------------------------------------
    void print() {
        std::cout << "V=[";
        bool first = true;
        for (int i=0; i<V.size(); i++) {
            if (solver->val(V[i]) == 1) {
                if (first) first=false; else std::cout << ",";
                std::cout << i;
            }
        }
        std::cout << "]\nE=[";
        first = true;
        for (int i=0; i<E.size(); i++) {
            if (solver->val(E[i]) == 1) {
                if (first) first=false; else std::cout << ",";
                std::cout << i;
            }
        }
        std::cout << "]"<<std::endl;
    }
};

} // namespace CaDiCaL
