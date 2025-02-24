#include <iostream>
#include <vector>
#include <fstream>
#include <regex>
#include <cmath>

#define EVEN 0
#define ODD  1

class Pool {
private:
    int id;
public:
    Pool() : id(0) {}
    int getId() { return ++id; }
    int top()   { return id; }
} pool;

//======================================================================================

class Game {
public:

    static const int DZN    = 0;
    static const int GM     = 1;

    std::vector<int>    owners;
    std::vector<int>    colors;
    std::vector<int>    sources;
    std::vector<int>    targets;
    std::vector<int>    V;
    std::vector<int>    E;
    int nvertices;
    int nedges;
    int start;
    std::vector<std::vector<std::vector<int>>>    P;
    std::vector<int>    oddcolors;

    //----------------------------------------------------------------

    void fixStartingZero() {
        for (int i=0; i<sources.size(); i++) {
            sources[i]--;
            targets[i]--;
        }
    }

    //----------------------------------------------------------------------------------

    void parseArray(const std::string& line,std::vector<int>& myvec) {
        std::regex pattern(R"(\[(.*?)\])");  // Regex to extract content inside [ ]
        std::smatch match;

        if (regex_search(line, match, pattern)) {
            std::string values = match[1];
            std::stringstream ss(values);
            std::string value;

            while (getline(ss, value, ',')) {
                myvec.push_back(stoi(value));
            }
        }
    }

    //----------------------------------------------------------------------------------

    void parseArray2(const std::string& line,std::vector<int>& vinfo, std::vector<int>& vedges, std::string& comment) {
        std::regex pattern(R"((\d+)\s+(\d+)\s+(\d+)\s+([\d,]+)\s+\"([^"]+)\")");
        std::smatch matches;
    
        std::sregex_iterator it(line.begin(), line.end(), pattern);
        std::sregex_iterator end;
    
        if (std::regex_match(line, matches, pattern)) {
            vinfo.push_back(std::stoi(matches[1]));
            vinfo.push_back(std::stoi(matches[2]));
            vinfo.push_back(std::stoi(matches[3]));
    
            // Convert list of integers
            std::stringstream ss(matches[4]);
            std::string num;
            while (std::getline(ss, num, ',')) {
                vedges.push_back(std::stoi(num));
            }
            comment = matches[5];
        }
    
    }

    //----------------------------------------------------------------------------------

    void calculate_oddcolors() {
        int sum = 0;
        for (int v = 0; v < nvertices; v++) if (colors[v]%2 == ODD) {
            if (!std::binary_search(oddcolors.begin(), oddcolors.end(), colors[v])) {
                auto pos = std::lower_bound(oddcolors.begin(), oddcolors.end(), colors[v]);
                oddcolors.insert(pos, colors[v]);
            }
        }
    }

    //----------------------------------------------------------------------------------

    int calculate_nbits(int n) {
        int sum = 0;
        for (int v = 0; v < nvertices; v++) if (colors[v] == n) sum++;
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
        std::vector<int> Q;
        std::vector<int> S;

        for (int i = 0; i < n+1; i++) {
            C.push_back(pool.getId());
        }
                
        for (int i = 0; i < n; i++) {
            Q.push_back(pool.getId());
            S.push_back(pool.getId());
        }

        cnf.push_back({-C[n], +A[n], -B[n]});
        cnf.push_back({+C[n], -A[n]});
        cnf.push_back({+C[n], +B[n]});

        for (int i = n-1; i >= 0; i--) {
            cnf.push_back({-Q[i], -A[i], +B[i]});
            cnf.push_back({-Q[i], +A[i], -B[i]});
            cnf.push_back({+Q[i], +A[i], +B[i]});
            cnf.push_back({+Q[i], -A[i], -B[i]});

            cnf.push_back({-S[i], +A[i]});
            cnf.push_back({-S[i], -B[i]});
            cnf.push_back({+S[i], -A[i], +B[i]});

            cnf.push_back({-C[i], +S[i], +Q[i  ]});
            cnf.push_back({-C[i], +S[i], +C[i+1]});
            cnf.push_back({+C[i], -S[i]         });
            cnf.push_back({+C[i], -Q[i], -C[i+1]});
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
        std::vector<int> Q;
        std::vector<int> S;


        for (int i = 0; i < n+1; i++) {
            C.push_back(pool.getId());
        }
                
        for (int i = 0; i < n; i++) {
            Q.push_back(pool.getId());
            S.push_back(pool.getId());
        }

        cnf.push_back({-C[n], +A[n]});
        cnf.push_back({-C[n], -B[n]});
        cnf.push_back({+C[n], -A[n], +B[n]});
        
        for (int i = n-1; i >= 0; i--) {
            cnf.push_back({-Q[i], -A[i], +B[i]});
            cnf.push_back({-Q[i], +A[i], -B[i]});
            cnf.push_back({+Q[i], +A[i], +B[i]});
            cnf.push_back({+Q[i], -A[i], -B[i]});

            cnf.push_back({-S[i], +A[i]});
            cnf.push_back({-S[i], -B[i]});
            cnf.push_back({+S[i], -A[i], +B[i]});

            cnf.push_back({-C[i], +S[i], +Q[i  ]});
            cnf.push_back({-C[i], +S[i], +C[i+1]});
            cnf.push_back({+C[i], -S[i]         });
            cnf.push_back({+C[i], -Q[i], -C[i+1]});
        }

        cnf.push_back({C[0]});

        return cnf;
    }    

    //----------------------------------------------------------------------------------

    void createLiterals() {
        calculate_oddcolors();

        for (int v = 0; v < nvertices; v++) {
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
        for (int e = 0; e < nedges; e++) E.push_back(pool.getId());
    }

    //----------------------------------------------------------------------------------

public:
    Game(std::string filename, int start, int type=DZN) : nvertices(0), nedges(0), start(start) {
        std::ifstream file(filename);
        if (!file) {
            std::cerr << "Error: Could not open file!" << std::endl;
            return;
        }

        std::string line;

        switch (type) {
            case DZN:
                while (getline(file, line)) {
                    if (line.find("nvertices") != std::string::npos) {
                        nvertices = stoi(line.substr(line.find("=") + 1));
                    } else if (line.find("nedges") != std::string::npos) {
                        nedges = stoi(line.substr(line.find("=") + 1));
                    } else if (line.find("owners") != std::string::npos) {
                        parseArray(line,owners);
                    } else if (line.find("colors") != std::string::npos) {
                        parseArray(line,colors);
                    } else if (line.find("sources") != std::string::npos) {
                        parseArray(line,sources);
                    } else if (line.find("targets") != std::string::npos) {
                        parseArray(line,targets);
                    }
                }
                file.close();
                break;

            case GM:
                int lastvertex = 0;
                std::vector<int> pos;
                int counter=0;
                while (getline(file, line)) {
                    if (line.find("parity") != std::string::npos) {
                        lastvertex = stoi(line.substr(line.find(" ")));
                        pos.reserve(lastvertex);
                    } else {
                        std::vector<int>    vinfo,vedges;
                        std::string         comment;
                        parseArray2(line,vinfo,vedges,comment);
                        pos[vinfo[0]] = counter++;
                    }
                }
                file.close();
                
                break;
        }

        fixStartingZero();

        createLiterals();
    }

    //----------------------------------------------------------------------------------

    Game( int levels, int blocks, int start ) : start(start) {
        nvertices   = ((blocks*3)+1)*(levels-1) + ((blocks*2)+1);
        nedges      = (blocks*6)*(levels-1) + (blocks*4) + (blocks*2*(levels-1));

        int es = 1;
        int os = 0;
        for (int l=1; l<levels; l++) {
            os = ((blocks*3)+1)*(levels-1)+1;
            for (int b=0; b<blocks; b++) {
                owners.push_back(1);
                owners.push_back(0);
                owners.push_back(0);
                colors.push_back(l*2);
                colors.push_back(l*2-1);
                colors.push_back(l*2);

                sources.push_back(es);   targets.push_back(es+1);
                sources.push_back(es);   targets.push_back(es+2);
                sources.push_back(es+1); targets.push_back(es+2);
                sources.push_back(es+2); targets.push_back(es);

                sources.push_back(es+2); targets.push_back(es+3);
                sources.push_back(es+3); targets.push_back(es+2);

                sources.push_back(es+2); targets.push_back(os+1);
                sources.push_back(os+1); targets.push_back(es+2);

                es += 3;
                os += 2;
            }
            owners.push_back(1);
            colors.push_back(l*2);
            es += 1;
        }
        int l = levels;
        for (int b=0; b<blocks; b++) {
            owners.push_back(0);
            owners.push_back(1);

            colors.push_back(l*2);
            colors.push_back(l*2-1);

            sources.push_back(es);   targets.push_back(es+1);
            sources.push_back(es+1); targets.push_back(es);
            sources.push_back(es+1); targets.push_back(es+2);
            sources.push_back(es+2); targets.push_back(es+1);
            
            es += 2;
        }
        owners.push_back(0);
        colors.push_back(l*2);

        fixStartingZero();
        createLiterals();
    }

    //----------------------------------------------------------------------------------

    std::vector<std::vector<int>> encode() {
        std::vector<std::vector<int>> cnf;
    
        // Starting vertex
        cnf.push_back({V[start]});
    
        // For every EVEN vertice, at least one outgoing edge must be activated
        for (int v=0; v<nvertices; v++) if (owners[v] == EVEN) {
            std::vector<int> clause;
            clause.push_back( -V[v] );
            for (int e=0; e<nedges; e++) if (sources[e]==v) {
                clause.push_back( E[e] );
            }
            cnf.push_back(clause);
        }
    
        // For every ODD vertice, each outgoing edge must be activated
        for (int v=0; v<nvertices; v++) if (owners[v] == ODD) {
            for (int e=0; e<nedges; e++) if (sources[e]==v) {
            std::vector<int> clause;
                clause.push_back( -V[v] );        
                clause.push_back( E[e] );
                cnf.push_back(clause);
            }
        }
    
        // For every activated edge, the source vertex must be activated
        for (int v=0; v<nvertices; v++) {
            for (int e=0; e<nedges; e++) if (sources[e]==v) {
                std::vector<int> clause;
                clause.push_back( -E[e] );
                clause.push_back( V[v] );
                cnf.push_back(clause);
            }
        }
    
        // For every activated edge, the target vertex must be activated
        for (int w=0; w<nvertices; w++) if (w != start) {
            for (int e=0; e<nedges; e++) if (targets[e]==w) {
                std::vector<int> clause;
                clause.push_back( -E[e] );
                clause.push_back( V[w] );
                cnf.push_back(clause);
            }
        }    
    
        // Progress measure constraints
        for (int e=0; e<nedges; e++) {
            int q = colors[targets[e]];
    
            std::vector<std::vector<int>> pm_cnf;
    
            for (auto& p : oddcolors) if (p < q) {
                std::vector<std::vector<int>> ge = encode_greaterequal(targets[e], sources[e], p);
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
    
                std::vector<std::vector<int>> ge = encode_strictlygreater(targets[e], sources[e], q);
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

//======================================================================================

int main(int argc, char const *argv[])
{
    // Game g("/home/chalo/Software/w-regular-games/data/game-SAT.dzn", 0);
    // Game g(2,1, 0);
    // auto cnf = g.encode();  // {{,},{,}}
    // dimacs(cnf, "/home/chalo/new.cnf");
    
    std::string line = "parity 563;";


    std::string num = line.substr(line.find(" "));
    int n = stoi(num);
    return 0;
}
