#include "iostream"
#include "cadical.hpp"
#include <queue>
#include <unordered_map>

#ifndef WINNING_CONDITIONS_H
#include "winning_conditions.h"
#endif

namespace CaDiCaL {

#define BoolSAT int
#define Lit int

//=============================================================================

class NoOpponentCycle : public ExternalPropagator {
private:
    Game& g;
    vec<BoolSAT>& V;
    vec<BoolSAT>& E;
    vec<int>    assigns;
    parity_type playerSAT;
    vec<WinningCondition*> conditions;

    size_t reasonLit;

    std::queue<int> propQueue;
    std::unordered_map<int, vec<int>> propReasons;

    vec<vec<int>> trail;

    int getValLit(int v){ return assigns[v]*v; }
    bool isFixed(int v) { return assigns[v] != 0; }
    bool isFalse(int v) { return assigns[v] == -1; }
    bool isTrue(int v)  { return assigns[v] == 1; }

public:
    NoOpponentCycle(Game& game, vec<BoolSAT>& V, vec<BoolSAT>& E, 
        parity_type playerSAT, vec<WinningCondition*> conditions)
    : g(game), V(V), E(E), playerSAT(playerSAT), conditions(conditions), 
        reasonLit(0), assigns(g.nvertices+g.nedges+1,0)
    {
        trail.push();
    }
    //-------------------------------------------------------------------------
    ~NoOpponentCycle() override = default;
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
        if (!propQueue.empty()) {
            int lit = propQueue.front();
            propQueue.pop();
            return lit;
        }

        vec<int> pathV;
        vec<int> pathE;

        filterEager(pathV,pathE,g.init,-1,true);

        if (!propQueue.empty()) {
            int lit = propQueue.front();
            propQueue.pop();
            return lit;
        }

        return 0;
    }
    //-------------------------------------------------------------------------
    int cb_add_reason_clause_lit(int propagated_lit) override {
        const vec<int>& reason = propReasons[propagated_lit];
        if (reasonLit < reason.size()) {
            int lit = reason[reasonLit++];
            return lit;
        }
        reasonLit = 0;
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
    bool filterEager(vec<int>& pathV, vec<int>& pathE, int v, 
        int lastEdge, bool definedEdge) 
    {
        int index = findVertex(v,pathV);
        if (index >= 0) {
            if (!satisfiedConditions(pathV,pathE,index)) {
                propReasons[-E[lastEdge]].clear();
                for (int i=0; i<pathV.size(); i++) {
                    propReasons[-E[lastEdge]].push( -E[pathE[i]] );
                }
                reasonLit = 0;
                
                propQueue.push(-E[lastEdge]);
                if (isTrue(E[lastEdge])) {
                    return true;
                }
                return false;
            }
        }
        else if (definedEdge) {
            pathV.push(v);
            for (int e : g.outs[v]) {
                if (isFalse(E[e])) continue;

                int w = g.targets[e];
                pathE.push(e);
                bool confl = filterEager(pathV, pathE, w, e, isTrue(E[e]));
                pathE.pop();
                if (confl) {
                    return true;
                }
            }
            pathV.pop();
        }
        return false;
    }
};

//=============================================================================

class NOCQModel {
private:
    Game& g;
    vec<BoolSAT> V;
    vec<BoolSAT> E;
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

            vec<BoolSAT> s(n-1);
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
        solver->statistics();
    }
};

} // namespace CaDiCaL
