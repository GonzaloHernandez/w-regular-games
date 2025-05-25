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

int findVertexReverse(int vertex,const std::vector<int>& path) {
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

// Overload + operator: vector + int → vector with int appended
std::vector<int> operator+(const std::vector<int>& v, int x) {
    std::vector<int> result = v;
    result.push_back(x);
    return result;
}

//-----------------------------------------------------------------------------------------------------

signed char getPlayBasic(Game& g, std::vector<int> path, int v) {
    int index = findVertex(v,path);
    if (index >= 0) {
        int best = bestcolor(g,index,path);
        return best%2;
    }
    else {
        int p = g.owners[v];
        for(auto& e : g.outs[v]) {
            std::vector <int> newpath = path;
            newpath.push_back(v);
            auto next = getPlayBasic(g, newpath, g.targets[e]);
            if (next == p) {
                return p;
            }
        }
        return 1-p;
    }
}

//-----------------------------------------------------------------------------------------------------

// signed char getPlayMemo(Game& g, std::vector<int> path, int v, signed char* memo) 
signed char getPlayMemo(Game& g, std::vector<int> path, int v, std::vector<int>& memo) 
{
    int index = findVertex(v,path);
    if (index >= 0) {
        int p = bestcolor(g,index,path)%2;
        if (p == g.owners[v]) {
            memo[v] = p;
            return p;
        }
        else {
            memo[v] = p+2;
            return p+2;
        }
    }
    else {
        int p = g.owners[v];
        bool uncertainty = false;
        for(auto& e : g.outs[v]) {
            int w = g.targets[e];
            if (memo[w] == p) {
                memo[v] = p;
                return p;
            }
            else if ((memo[w]==2 || memo[w]==3) && (memo[v]==2 || memo[v]==3)) {
                if (memo[w]%2 == p) {
                    memo[v] = p;
                    return p;                    
                }
                else {
                    uncertainty = true;
                }
            }
            else {
                auto next = getPlayMemo(g, path+v, g.targets[e],memo);
                if (next == p) {
                    memo[v] = p;
                    return p;
                }
                else if ((next==2 || next==3) && (memo[v]==2 || memo[v]==3)) {
                    if (next%2 == p) {
                        memo[v] = p;
                        return p;                    
                    }
                    else {
                        uncertainty = true;
                    }
                }
            }
        }
        if ((memo[v]==2 || memo[v]==3) && !uncertainty) {
            memo[v] = 1-p;
            return 1-p;
        }
        else {
            memo[v] = (1-p)+2;
            return (1-p)+2;
        }
    }
}

//-----------------------------------------------------------------------------------------------------

signed char getPlay(Game& g, int p, int start, bool basic) {
    if (basic) {
        return getPlayBasic(g, {}, start);
    }

    // std::unique_ptr<signed char[]> memo = std::make_unique<signed char[]>(g.nvertices);
    // std::fill_n(memo.get(), g.nvertices, 9);
    // auto play = getPlayMemo(g, {}, start, memo.get());

    std::vector<int> memo(g.nvertices,9);   
    auto play = getPlayMemo(g, {}, start, memo);

    return play%2;
}
