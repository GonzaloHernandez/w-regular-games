#include "hra.h"

#define loop first
#define best second

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

int bestcolor(Game& g, int index,std::vector<int>& path){
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

signed char getPlayMemo(Game& g, std::vector<int> path, int v, int d, 
                        std::vector<std::pair<int,int>>& memoV,
                        std::vector<std::pair<int,int>>& memoE ) 
{
    int index = findVertex(v,path);
    if (index >= 0) {
        int best = bestcolor(g,index,path);
        memoE[d].loop   = v;
        memoE[d].best   = best;
        return best%2;
    }
    else {
        for(auto& e : g.vedges[v]) {
            int w = g.targets[e];
            if (memoE[e].loop < 0) {
                std::vector <int> newpath = path;
                newpath.push_back(v);
                auto parity = getPlayMemo(g, newpath, w, e, memoV, memoE);
                memoV[v] = memoE[e]; 
                if (parity == g.owners[v]) {
                    return parity;
                }
                continue;
            }
            int index = findVertex(memoE[e].loop,path);
            if (index < 0) {
                memoV[v] = memoE[e];
                if (memoE[e].best%2 == g.owners[v]) {
                    return memoV[v].best%2;
                }
                continue;
            }

            int best = bestcolor(g,index,path);
            if (!((g.reward==MIN && best < memoE[e].best) || (g.reward==MAX && best > memoE[e].best))) {
                best = memoE[e].best;
            }

            memoV[v].loop   = memoE[e].loop;
            memoV[v].best   = best;
            if (best%2 == g.owners[v]) {
                return best%2;
            }
        }
        return 1-g.owners[v];
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
    std::vector<std::pair<int,int>> memoV(g.nvertices,   {-1,-1});
    std::vector<std::pair<int,int>> memoE(g.nedges,      {-1,-1});

    return getPlayMemo(g, path, start, -1, memoV, memoE);
}
