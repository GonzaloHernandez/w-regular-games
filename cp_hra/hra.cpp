#include "hra.h"

int findVertex(int vertex,std::vector<int>& path) {
    for (int i=0; i<path.size(); i++) {
        if (path[i] == vertex) return i;
    }
    return -1;
}

int bestcolor(Game& g, int index,std::vector<int>& path){
    int m = g.colors[path[index]];
    for (int i=index+1; i<path.size(); i++) {
        if (g.reward==MIN && g.colors[path[i]] < m) {
            m = g.colors[path[i]];
        }
        else if (g.reward==MAX && g.colors[path[i]] > m) {
            m = g.colors[path[i]];
        }
    }
    return m;
}

signed char getPlay(Game& g, int p, std::vector<int> path, int current) {
    int index = findVertex(current,path);
    if (index >= 0) {
        int best = bestcolor(g,index,path);
        return best%2;
    }
    else {
        if (g.owners[current] == p) {
            for(auto& e : g.vedges[current]) {
                std::vector <int> newpath = path;
                newpath.push_back(current);
                auto next = getPlay(g, p, newpath, g.targets[e]);
                if (next == p) {
                    return p;
                }
            }
            return 1-p;
        }
        else {
            return getPlay(g, 1-p , path, current);
        }
    }
}

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

signed char getPlayMemo(Game& g, int p, std::vector<int> path, int current, std::vector<int>& memo) {
    int index = findVertex(current,path);
    if (index >= 0) {
        int best = bestcolor(g,index,path);
        return best%2;
    }
    else {
        int np = g.owners[current];
        for(auto& e : g.vedges[current]) {
            if (memo[g.targets[e]] == np) {
                memo[current] = np;
                return np; // using this edge current can force np (already memoized)
            }

            if (memo[g.targets[e]] == 1-np) {
                continue; // skip this edge
            }

            std::vector <int> newpath = path;
            newpath.push_back(current);
            auto next = getPlayMemo(g, np, newpath, g.targets[e], memo);
            if (next == np) {
                memo[current] = np;
                return np; // using this edge current can force np 
            }
        }
        memo[current] = 1-np;
        return 1-np;
    }
}
