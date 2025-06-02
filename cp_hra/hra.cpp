#include "hra.h"

struct splay {
    int loop;
    int best;
    int parity;
    bool touched()  { return loop>=0; }
};

//-----------------------------------------------------------------------------------------------------

// Overload + operator: vector + int → vector with int appended
std::vector<int> operator+(const std::vector<int>& v, int x) {
    std::vector<int> result = v;
    result.push_back(x);
    return result;
}

//-----------------------------------------------------------------------------------------------------

int findVertex(int vertex,const std::vector<int>& path) {
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
splay getPlayMemo(Game& g, std::vector<int> path, int v, 
                        std::vector<splay>& memo) 
{
    splay next;
    int p = g.owners[v];
    for(auto& e : g.outs[v]) {
        int w = g.targets[e];

        int best;
        int index = findVertex(w,path+v);
        if (index >= 0) {
            best = bestcolor(g,index,path+v);
            next = {w, g.colors[v], best%2};
        }
        else if (memo[e].touched()) {
            int index = findVertex(memo[e].loop,path+v);
            if (index >= 0) {
                best = memo[e].best;
            }
            else {
                next = getPlayMemo(g, path+v, w, memo);
            }
        }
        else {
            next = getPlayMemo(g, path+v, w, memo);
        }

        if (next.loop != v) {
            if ((g.reward==MIN && g.colors[w] < next.best) || 
                (g.reward==MAX && g.colors[w] > next.best)) 
            {
                memo[e] = {next.loop, g.colors[w], next.parity};
            }
            else {
                memo[e] = next;
            }
        }

        if (next.parity == p) {
            return next;
        }
    }
    return next;
}

//-----------------------------------------------------------------------------------------------------

signed char getPlay(Game& g, int start, bool basic) {
    if (basic) {
        return getPlayBasic(g, {}, start);
    }

    // std::unique_ptr<signed char[]> memo = std::make_unique<signed char[]>(g.nvertices);
    // std::fill_n(memo.get(), g.nvertices, 9);
    // auto play = getPlayMemo(g, {}, start, memo.get());

    std::vector<splay> memo(g.nedges,{-1,-1,-1});   
    auto play = getPlayMemo(g, {}, start, memo);

    return play.parity;
}
