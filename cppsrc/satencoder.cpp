#ifndef CPP_GAME
#include "game.cpp"
#endif

#include <cmath>
#include <chrono>

//======================================================================================

class Pool {
private:
    int id;
public:
    Pool() : id(0) {}
    int getId() { return ++id; }
    int top()   { return id; }
} pool;

//======================================================================================

class SATEncoder {
private:
    Game& g;
    std::vector<int>    V;
    std::vector<int>    E;
    std::vector<std::vector<std::vector<int>>>    P;
    std::vector<int>    oddcolors;

    //----------------------------------------------------------------------------------

    void calculate_oddcolors() {
        int sum = 0;
        for (int v = 0; v < g.nvertices; v++) if (g.colors[v]%2 == ODD) {
            if (!std::binary_search(oddcolors.begin(), oddcolors.end(), g.colors[v])) {
                auto pos = std::lower_bound(oddcolors.begin(), oddcolors.end(), g.colors[v]);
                oddcolors.insert(pos, g.colors[v]);
            }
        }
    }

    //----------------------------------------------------------------------------------

    int calculate_nbits(int n) {
        int sum = 0;
        for (int v = 0; v < g.nvertices; v++) if (g.colors[v] == n) sum++;
        return std::ceil(std::log2(sum + 1));
    }

    //----------------------------------------------------------------------------------

    std::vector<std::vector<int>> encode_greaterequal(int a, int b, int p) {
        std::vector<std::vector<int>> cnf;

        int n = calculate_nbits(p)-1;

        auto it = std::find(oddcolors.begin(), oddcolors.end(), p);
        p = std::distance(oddcolors.begin(), it);

        std::vector<int>& A = P[a][p];
        std::vector<int>& B = P[b][p];
        std::vector<int> C;

        for (int i = 0; i < n+1; i++) {
            C.push_back(pool.getId());
        }

        cnf.push_back({-C[n], +A[n], -B[n]});
        cnf.push_back({+C[n], -A[n]});
        cnf.push_back({+C[n], +B[n]});

        for (int i = n-1; i >= 0; i--) {
            cnf.push_back({-C[i], -B[i], +A[i]});
            cnf.push_back({-C[i], +A[i], +C[i+1]});
            cnf.push_back({-C[i], -B[i], +C[i+1]});
    
            cnf.push_back({+C[i], +B[i], -A[i]});
            cnf.push_back({+C[i], +B[i], -A[i], -C[i+1]});
            cnf.push_back({+C[i], +B[i], -C[i+1]});
            cnf.push_back({+C[i], -A[i], -C[i+1]});
        }

        cnf.push_back({C[0]});

        return cnf;
    } 

    //----------------------------------------------------------------------------------

    std::vector<std::vector<int>> encode_strictlygreater(int a, int b, int p) {
        std::vector<std::vector<int>> cnf;

        int n = calculate_nbits(p)-1;

        auto it = std::find(oddcolors.begin(), oddcolors.end(), p);
        p = std::distance(oddcolors.begin(), it);

        std::vector<int>& A = P[a][p];
        std::vector<int>& B = P[b][p];
        std::vector<int> C;

        for (int i = 0; i < n+1; i++) {
            C.push_back(pool.getId());
        }
                
        cnf.push_back({-C[n], +A[n]});
        cnf.push_back({-C[n], -B[n]});
        cnf.push_back({+C[n], -A[n], +B[n]});
        
        for (int i = n-1; i >= 0; i--) {
            cnf.push_back({-C[i], -B[i], +A[i]});
            cnf.push_back({-C[i], +A[i], +C[i+1]});
            cnf.push_back({-C[i], -B[i], +C[i+1]});
    
            cnf.push_back({+C[i], +B[i], -A[i]});
            cnf.push_back({+C[i], +B[i], -A[i], -C[i+1]});
            cnf.push_back({+C[i], +B[i], -C[i+1]});
            cnf.push_back({+C[i], -A[i], -C[i+1]});
        }

        cnf.push_back({C[0]});

        return cnf;
    }    

    //----------------------------------------------------------------------------------

    void createLiterals() {
        calculate_oddcolors();

        for (int v = 0; v < g.nvertices; v++) {
            V.push_back(pool.getId());
            std::vector<std::vector<int>> ps;
            for(int p = 0; p < oddcolors.size(); p++) {
                std::vector<int> ms;
                for (int m = 0; m < calculate_nbits(oddcolors[p]); m++) {
                    ms.push_back(pool.getId());
                }
                ps.push_back(ms);
            }
            P.push_back(ps);
        }
        for (int e = 0; e < g.nedges; e++) E.push_back(pool.getId());
    }

    //----------------------------------------------------------------------------------

public:
    SATEncoder(Game& g) : g(g) {
        createLiterals();
    }
    //----------------------------------------------------------------------------------

    std::vector<std::vector<int>> getCNF() {
        std::vector<std::vector<int>> cnf;
    
        // Starting vertex
        cnf.push_back({V[g.start]});
    
        // For every EVEN vertice, at least one outgoing edge must be activated
        for (int v=0; v<g.nvertices; v++) if (g.owners[v] == EVEN) {
            std::vector<int> clause;
            clause.push_back( -V[v] );
            for (int e=0; e<g.nedges; e++) if (g.sources[e]==v) {
                clause.push_back( E[e] );
            }
            cnf.push_back(clause);
        }
    
        // For every ODD vertice, each outgoing edge must be activated
        for (int v=0; v<g.nvertices; v++) if (g.owners[v] == ODD) {
            for (int e=0; e<g.nedges; e++) if (g.sources[e]==v) {
            std::vector<int> clause;
                clause.push_back( -V[v] );        
                clause.push_back( E[e] );
                cnf.push_back(clause);
            }
        }
    
        // For every activated edge, the source vertex must be activated
        for (int v=0; v<g.nvertices; v++) {
            for (int e=0; e<g.nedges; e++) if (g.sources[e]==v) {
                std::vector<int> clause;
                clause.push_back( -E[e] );
                clause.push_back( V[v] );
                cnf.push_back(clause);
            }
        }
    
        // For every activated edge, the target vertex must be activated
        for (int w=0; w<g.nvertices; w++) if (w != g.start) {
            for (int e=0; e<g.nedges; e++) if (g.targets[e]==w) {
                std::vector<int> clause;
                clause.push_back( -E[e] );
                clause.push_back( V[w] );
                cnf.push_back(clause);
            }
        }    
    
        // Progress measure constraints
        for (int e=0; e<g.nedges; e++) {
            int q = g.colors[g.targets[e]];
    
            std::vector<std::vector<int>> pm_cnf;
    
            for (auto& p : oddcolors) if (p < q) {
                std::vector<std::vector<int>> ge = encode_greaterequal(g.targets[e], g.sources[e], p);
                pm_cnf.insert(pm_cnf.end(), ge.begin(), ge.end());
            }
    
            if (q % 2 == EVEN) {
                for (auto& clause : pm_cnf) {
                    std::vector<int> new_clause = clause;
                    new_clause.push_back(-E[e]);
                    cnf.push_back(new_clause);
                }
            }
            else {
                for (auto& clause : pm_cnf) {
                    std::vector<int> new_clause = clause;
                    new_clause.push_back(-E[e]);
                    cnf.push_back(new_clause);
                }
    
                std::vector<std::vector<int>> ge = encode_strictlygreater(g.targets[e], g.sources[e], q);
                for (auto& clause : ge) {
                    std::vector<int> new_clause = clause;
                    new_clause.push_back(-E[e]);
                    cnf.push_back(new_clause);
                }
            }
        }
    
        return cnf;
    }
    
};

//======================================================================================

void dimacs(std::vector<std::vector<int>>& cnf, std::string filename) {
    std::ofstream file(filename);
    if (!file) {
        std::cerr << "Error: Could not open file!" << std::endl;
        return;
    }

    file << "p cnf " << pool.top() << " " << cnf.size() << std::endl;
    for (auto& clause : cnf) {
        for (auto& literal : clause) {
            file << literal << " ";
        }
        file << "0" << std::endl;
    }
    file.close();
}
