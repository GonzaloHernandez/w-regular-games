#include <vector>
#include <unordered_set>
#include <algorithm>
#include "game.cpp"

class GameExtended : public Game {
public:
    std::vector<int>    vertices;
    std::vector<int>    edges;
public:

    GameExtended(   std::vector<int> own,std::vector<int> col,
                    std::vector<int> sou,std::vector<int> tar, int startv)
    : Game(own, col, sou, tar, startv)
    {
        for(int v=0; v<=nvertices; v++) vertices.push_back(v);
        for(int e=0; e<=nedges;    e++) edges.push_back(e);
    }

    //----------------------------------------------------------------------------------
    
    GameExtended(std::string filename, int start, int type=DZN)
    : Game(filename,start,type)
    {
        for(int v=0; v<=nvertices; v++) vertices.push_back(v);
        for(int e=0; e<=nedges;    e++) edges.push_back(e);
    }

    //----------------------------------------------------------------------------------

    GameExtended(int levels, int blocks, int start )
    : Game(levels, blocks, start) 
    {
        for(int v=0; v<=nvertices; v++) vertices.push_back(v);
        for(int e=0; e<=nedges;    e++) edges.push_back(e);
    }

    //----------------------------------------------------------------------------------

    GameExtended& GameExtended(GameExtended& g) {
        GameExtended ng(g.owners,g.colors,g.sources,g.targets,g.start);
    }

    //----------------------------------------------------------------------------------

    void printGame() {
        Game::printGame();
        std::cout << "vertices:  {";
        for(int v=0; v<owners.size(); v++) 
            std::cout<<(vertices[v]>=0?std::to_string(vertices[v]):" ")<<(v<vertices.size()-1?",":"");
        std::cout << "}" << std::endl;
        std::cout << "edges:     {";
        for(int e=0; e<edges.size(); e++) 
            std::cout<<(edges[e]>=0?std::to_string(edges[e]):" ")<<(e<edges.size()-1?",":"");
        std::cout << "}" << std::endl;

    }

    //----------------------------------------------------------------------------------

    std::vector<int> getVertices() {
        std::vector<int> vs;
        for(int v=0; v<vertices.size(); v++) if (vertices[v]>=0) {
            vs.push_back(v);
        }
        return vs;
    }

    //----------------------------------------------------------------------------------

    std::vector<int> getEdges() {
        std::vector<int> es;
        for(int e=0; e<edges.size(); e++) if (edges[e]>=0) {
            es.push_back(e);
        }
        return es;
    }
    //----------------------------------------------------------------------------------
    
    GameExtended operator/(const std::vector<int>& novertices) {
        GameExtended g(owners,colors,sources,targets,start);
        for (auto& v : novertices) {
            g.vertices[v] = -1;
            for (auto& e : edges) {
                if (e >= 0) 
                    if (g.sources[e] == v or g.targets[e] == v) 
                        g.edges[e] = -1;
            }
        }
        return g;
    }
};
    
std::vector<int> attractor(std::vector<int> V, int q, const GameExtended& g) {
    std::vector<int> A = V;
    for (int v : V) {
        std::vector<int> ownvertices, othervertices, novertices;
        
        for (int e=0; e<g.nedges; e++) if (g.edges[e]>=0) {
            if (g.targets[e] == v) {
                if (g.owners[g.sources[e]] == q) {
                    ownvertices.push_back(g.sources[e]);
                } else {
                    othervertices.push_back(g.sources[e]);
                }
            }
        }
        
        for (int e=0; e<g.nedges; e++) if (g.edges[e]>=0) {
            if (g.owners[g.sources[e]] != q 
            && std::find(othervertices.begin(),othervertices.end(),g.sources[e]) != othervertices.end() 
            && std::find(V.begin(),V.end(),g.targets[e]) == V.end()) {
                novertices.push_back(g.sources[e]);
            }
        }
        
        std::vector<int> localattractor = ownvertices;
        for (int v : othervertices) {
            if (std::find(novertices.begin(),novertices.end(),v) != novertices.end()) {
                localattractor.push_back(v);
            }
        }
        
        auto recursiveAttr = attractor(localattractor, q, g / A );
        for (int v : recursiveAttr) {
            if (std::find(A.begin(),A.end(),v) != A.end()) {
                A.push_back(v);
            }
        }
    }
    return A;
}

std::vector<std::vector<int>> zielonka(GameExtended g) {
    std::vector<std::vector<int>> Z(2);
    if (g.getVertices().empty()) {
        return { {}, {} };
    } else {
        int m = *std::min_element(g.colors.begin(), g.colors.end());
        int q = m % 2;
        std::vector<int> U;
        
        for (int v : g.getVertices()) {
            if (g.colors[v] == m) {
                U.push_back(v);
            }
        }
        
        std::vector<int> A = attractor(U, q, g);
        auto W = zielonka(g / A);
        
        if (W[1 - q].empty()) {
            Z[q] = A;
        } else {
            std::vector<int> B = attractor(W[1-q], 1 - q, g);
            W = zielonka(g / B);
            Z[q] = W[q];
            Z[1-q] = B;
            for (int v : W[1-q]) {
                if (std::find(B.begin(),B.end(),v) == B.end())
                    Z[1-q].push_back(v);
            }
        }
    }
    return Z;
}

int main(int argc, char *argv[]) {
    GameExtended g("/home/chalo/Software/w-regular-games/data/game-wiki-min.dzn",Game::DZN);
    std::vector<std::vector<int>>  W = zielonka(g);
    for (auto& v : W[0]) std::cout << v;
    std::cout << " - ";
    for (auto& v : W[1]) std::cout << v;
    std::cout << std::endl;

    // GameExtended g({0,1},{0,1},{0,1},{1,0},0);
    // g.printGame();
}