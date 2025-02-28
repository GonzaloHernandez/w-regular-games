#include "game.cpp"
#include <iostream>
#include <vector>

#define vec(T) std::vector<T>

vec(int) attractor(vec(int)& V, int q, Game& g) {
    vec(int) A = V;
    for(auto v : V) {
        vec(int) ownvertices;
        for (int e=0; e<g.nedges; e++) if (g.targets[e]==v && g.owners[g.sources[e]]==q) {
            ownvertices.push_back( g.sources[e] );
        }
        vec(int) othervertices;
        for (int e=0; e<g.nedges; e++) if (g.targets[e]==v && g.owners[g.sources[e]]!=q) {
            othervertices.push_back( g.sources[e] );
        }
        vec(int) novertices;
        for (int e=0; e<g.nedges; e++) {
            if (g.owners[g.sources[e]]!=q 
            &&  std::find(othervertices.begin(), othervertices.end(), g.sources[e]) != othervertices.end()
            &&  std::find(V.begin(), V.end(), g.targets[e]) != V.end()) 
            {
                novertices.push_back( g.sources[e] );
            }
        }
        vec(int) localattractor = ownvertices;
        
    }
    return A;
}

int main(int argc, char* argv[]) {
    std::cout<<"*** Hi ***"<<std::endl;
}
