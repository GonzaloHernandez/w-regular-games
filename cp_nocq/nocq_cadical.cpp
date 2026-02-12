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
    vec<BoolView>&  vars;
    vec<int>        assigns;

    vec<int>        reason;
    size_t          index;

    int getValLit(int v){ return assigns[v]*v; }
    bool isFixed(int v) { return assigns[v] != 0; }
    bool isFalse(int v) { return assigns[v] == -1; }
    bool isTrue(int v)  { return assigns[v] == 1; }
public:
    Different(vec<BoolView>& vars) 
    : vars(vars), assigns(vars.size()+1, 0), index(0) 
    {}
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
    int checker() {
        for (int i=0; i<vars.size(); i++) if (!isFixed(vars[i])) return 0;

        for (int i=0; i<vars.size()-1; i++) {
            if (isTrue(vars[i]) == isTrue(vars[i+1])) {
                reason.clear();
                reason.push(-getValLit(vars[i]));
                reason.push(-getValLit(vars[i+1]));
                reason.push(0);
                index = 0; 
                return -getValLit(vars[i]);
            }
        }

        return 0;
    }
    //-------------------------------------------------------------------------
    int filter() {
        for (int i=0; i<vars.size()-1; i++) { int k=i+1;
            if (!isFixed(vars[i]) && !isFixed(vars[k])) continue;

            if (isFixed(vars[i]) && isFixed(vars[k]) && 
                isTrue(vars[i]) != isTrue(vars[k])) continue;

            if (isFixed(vars[i])) {                
                reason.clear();
                reason.push(isTrue(vars[i]) ? -vars[i] : vars[i]);
                reason.push(isTrue(vars[i]) ? -vars[k] : vars[k]);
                reason.push(0);
                index = 0;
                return isTrue(vars[i]) ? -vars[k] : vars[k];
            } else {
                reason.push(isTrue(vars[k]) ? -vars[i] : vars[i]);
                reason.push(isTrue(vars[k]) ? -vars[k] : vars[k]);
                reason.push(0);
                index = 0;
                return isTrue(vars[k]) ? -vars[i] : vars[i];
            }
        }
        return 0;
    }
    //-------------------------------------------------------------------------
    int cb_propagate() override {
        // return checker();
        return filter();
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

class CadModel {
private:
    int n;
    vec<BoolView> vars;
    int pool=0;
    Solver* solver;

    int newBoolVar ()           { pool += 1;    return pool; }
    int newBoolVars(int size)   { pool += size; return pool-size+1; }
public:
    //-------------------------------------------------------------------------
    CadModel(int n) : n(n) {
        solver = new Solver();
        solver->set("factor",0);

        vars.growTo(n,0);
        for (int i=0; i<n; i++) vars[i] = newBoolVar();
        for (int i=0; i<n; i++) solver->add(vars[i]); solver->add(0);

        // solver->clause(-vars[0]);
        // solver->clause(-vars[1]);
        // solver->clause(-vars[2]);
        solver->connect_external_propagator(new Different(vars));
        for (int i=0; i<n; i++) solver->add_observed_var(vars[i]);
    }
    //-------------------------------------------------------------------------
    ~CadModel() {
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
        std::cout << "vars=[";
        for (int i=0; i<n; i++) {
            if (i>0) std::cout << ",";
            std::cout << solver->val(vars[i]);
        }
        std::cout << "]" << std::endl;
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

    vec<Lit> reason;
    size_t reasonLit;

    vec<vec<int>> trail;

    int getValLit(int v){ return assigns[v]*v; }
    bool isFixed(int v) { return assigns[v] != 0; }
    bool isFalse(int v) { return assigns[v] == -1; }
    bool isTrue(int v)  { return assigns[v] == 1; }

public:
    NoOpponentCycle(Game& game, vec<BoolView>& V, vec<BoolView>& E, 
        parity_type playerSAT, vec<WinningCondition*> conditions)
    : g(game), V(V), E(E), playerSAT(playerSAT), conditions(conditions), 
        reasonLit(0), assigns(g.nvertices+g.nedges+1,0)
    {
        trail.push();
    }
    //-------------------------------------------------------------------------
    void notify_assignment(const std::vector<int> &lits) override {
        for (int lit : lits) {
            int v = abs(lit);
            if (assigns[v] != 0) continue;
            assigns[v] = (lit > 0) ? 1 : -1;
            trail.last().push(v);
        }
    }
    //-------------------------------------------------------------------------
    void notify_new_decision_level () override {
        trail.push();
    }
    //-------------------------------------------------------------------------
    void notify_backtrack(size_t new_level) override {
        while (trail.size() > (new_level+1)) {
            for (int i=0; i<trail.last().size(); i++) { 
                int v = trail.last()[i];
                assigns[v] = 0;
            }
            trail.pop();
        }
    }
    //-------------------------------------------------------------------------
    bool cb_check_found_model (const std::vector<int> &model) override {
        return true;
    }
    //-------------------------------------------------------------------------
    int cb_decide() override {
        return 0;
    }
    //-------------------------------------------------------------------------
    int cb_propagate() override {
        vec<int> pathV;
        vec<int> pathE;
        reason.clear(); reasonLit = 0;

        return filterEager(pathV,pathE,g.init,-1,true);
    }
    //-------------------------------------------------------------------------
    int cb_add_reason_clause_lit(int propagated_lit) override {
        if (reasonLit < reason.size()) {
            int lit = reason[reasonLit++];
            return lit;
        }
        return 0;
    }
    //-------------------------------------------------------------------------
    bool cb_has_external_clause(bool &is_forgettable) override {
        return false;
    }
    //-------------------------------------------------------------------------
    int cb_add_external_clause_lit() override {
        return 0;
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
        for (int i=from; i<path.size(); i++) {
            lits.push( -B[path[i]] );
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
                clausify(pathE,E,reason,0);
                reasonLit = 0;
                return -E[lastEdge];
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
                if (status != 0) {
                    return status;
                }
            }
            pathV.pop();
        }
        return 0;
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

            {                           // V -> E_0 \/ E_1 \/ ... \/ E_n
                solver->add( -V[v] );
                for (int e : g.outs[v]) solver->add( E[e] );
                solver->add(0); 
            }
            
            // --- At most one ------------------------------------------------
            if (n == 1) continue;

            vec<BoolView> s(n-1);
            for (int j = 0; j < n-1; j++) s[j] = newBoolVar();

            // First literal
            {
                int e = g.outs[v][0];
                solver->clause( -E[e], s[0] );      // E_0 -> s_0
            }

            // Middle literals
            for (int i = 1; i < n-1; i++) {
                int e = g.outs[v][i];
                solver->clause( -E[e], s[i] );      // E_i -> s_i
                solver->clause( -E[e], -s[i-1] );   // E_i -> -s_{i-1}
                solver->clause( -s[i-1], s[i] );    // s_{i-1} -> s_i
            }

            // Last literal
            {
                int e = g.outs[v][n-1];
                solver->clause( -E[e], -s[n-2] );   // E_{n-1} -> -s_{n-2}
            }
        }
    
        // --------------------------------------------------------------------
        // For every active OPPONENT vertice, each outgoing edge must be activated
        for (int v=0; v<g.nvertices; v++) if (g.owners[v]==opponent(playerSAT)) {
            for (int e : g.outs[v]) {
                solver->clause( -V[v], E[e] );      // V -> E
            }
        }

        // --------------------------------------------------------------------
        // For every active edge, the target vertex must be activated
        for (int e=0; e<g.nedges; e++) {
            int w = g.targets[e];
            solver->clause( -E[e], V[w] );          // E -> V
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
