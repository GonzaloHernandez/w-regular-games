#include "hra.h"

struct splay {
    int loop;
    int best;
    bool parity()   { return best%2; }
    bool touched()  { return loop>=0; }
};

//-----------------------------------------------------------------------------------------------------

int findVertex(int vertex,std::vector<int>& path) {
    for (int i=0; i<path.size(); i++) {
        if (path[i] == vertex) return i;
    }
    return -1;
}

//-----------------------------------------------------------------------------------------------------

int findVertexReverse(int vertex,std::vector<int>& path) {
    for (int i=path.size()-1; i>=0; i--) {
        if (path[i] == vertex) return i;
    }
    return -1;
}

//-----------------------------------------------------------------------------------------------------

int bestcolor(Game& g, int index,const std::vector<int>& path){
    int m = g.colors[path[index]];
    for (int i=index+1; i<path.size(); i++) {
        if ((g.reward==MIN && g.colors[path[i]] < m) || (g.reward==MAX && g.colors[path[i]] > m)) {
            m = g.colors[path[i]];
        }
    }
    return m;
}

//-----------------------------------------------------------------------------------------------------

signed char getPlayBasic(Game& g, std::vector<int> path, int current) {
    int index = findVertex(current,path);
    if (index >= 0) {
        int best = bestcolor(g,index,path);
        return best%2;
    }
    else {
        int p = g.owners[current];
        for(auto& e : g.vedges[current]) {
            std::vector <int> newpath = path;
            newpath.push_back(current);
            auto next = getPlayBasic(g, newpath, g.targets[e]);
            if (next == p) {
                return p;
            }
        }
        return 1-p;
    }
}

//-----------------------------------------------------------------------------------------------------

// signed char getPlayMemo(Game& g, int p, std::vector<int> path, int current, std::vector<int>& memo) {
//     int index = findVertex(current,path);
//     if (index >= 0) {
//         int best = bestcolor(g,index,path);
//         return best%2;
//     }
//     else {
//         if (g.owners[current] == p) {
//             for(auto& e : g.vedges[current]) {
//                 if (memo[g.targets[e]] == p) {
//                     memo[current] = p;
//                     return p; // with this edge current can force p (already memoized)
//                 }

//                 if (memo[g.targets[e]] == 1-p) {
//                     continue; // skip this edge
//                 }

//                 std::vector <int> newpath = path;
//                 newpath.push_back(current);
//                 auto next = getPlayMemo(g, p, newpath, g.targets[e], memo);
//                 if (next == p) {
//                     memo[current] = p;
//                     return p; // with this edge current can force p
//                 }
//             }
//             memo[current] = 1-p;
//             return 1-p;
//         }
//         else {
//             return getPlayMemo(g, 1-p , path, current, memo);
//         }
//     }
// }

//-----------------------------------------------------------------------------------------------------

// Overload + operator: vector + int → vector with int appended
std::vector<int> operator+(const std::vector<int>& v, int x) {
    std::vector<int> result = v;
    result.push_back(x);
    return result;
}

//-----------------------------------------------------------------------------------------------------

splay getPlayMemo( Game& g, std::vector<int> path, int v, int d, std::vector<splay>& memo) 
{
    int index = findVertex(v,path);
    if (index >= 0) {
        int best = bestcolor(g,index,path);
        return splay{v,best};
    }
    else {
        splay play;
        for(auto& e : g.vedges[v]) {
            int w = g.targets[e];
            
            if (!memo[w].touched()) {
                play = getPlayMemo(g, path+v, w, e, memo);
                if (play.parity() == g.owners[v]) {
                    memo[v] = play;
                    return play;
                }
                continue;
            }

            int index = findVertexReverse(memo[w].loop,path);
            if (index < 0) {
                play = memo[w];
                if (play.parity() == g.owners[v]) {
                    memo[v] = play;
                    return play;
                }
                continue;
            }

            int best = bestcolor(g,index,path+v);
            if (!((g.reward==MIN && best < memo[w].best) || (g.reward==MAX && best > memo[w].best))) {
                best = memo[w].best;
            }
            play = {memo[w].loop,best};
            if (play.parity() == g.owners[v]) {
                memo[v] = play;
                return play;
            }
        }
        return play;
    }
}

//-----------------------------------------------------------------------------------------------------

// signed char getPlayMemo(Game& g, int p, std::vector<int> path, int current, 
//                         std::vector<int>& memo,std::vector<signed char>& parities) 
// {
//     int index = findVertexReverse(current,path);
//     if (index >= 0) {
//         int best = bestcolor(g,index,path);
//         if (best%2 == g.owners[current]) 
//             return best%2;
//         else 
//             path.resize(index);
//     }

//     int np = g.owners[current];
    
//     while (memo[current] < g.vedges[current].size()) {
//         int e = g.vedges[current][memo[current]];
//         memo[current]++;

//         if (parities[g.targets[e]] == np) {
//             parities[current] = np;
//             return np; // using this edge current can force np (already memoized)
//         }

//         if (parities[g.targets[e]] == 1-np) {
//             continue; // skip this edge
//         }

//         std::vector <int> newpath = path;
//         newpath.push_back(current);
//         auto nextp = getPlayMemo(g, np, newpath, g.targets[e], memo, parities);
//         if (nextp == np) {
//             parities[current] = nextp;
//             return nextp; // using this edge current can force np 
//         }
//     }
//     parities[current] = 1-np;
//     return 1-np;
// }

//-----------------------------------------------------------------------------------------------------

signed char getPlay(Game& g, int p, int start) {
    std::vector<int> path;
    std::vector<splay> memo(g.nvertices, {-1,-1});

    splay play = getPlayMemo(g, path, start, -1, memo);
    return play.parity();
}
