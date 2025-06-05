#include "hra.h"

struct splay {
    int loop;
    int best;
    int from;
    bool touched()  { return loop>=0; }
    bool parity()   { return best%2; }
};

//-----------------------------------------------------------------------------------------------------

std::vector<int> operator+(const std::vector<int>& v, int x) {
    std::vector<int> result = v;
    result.push_back(x);
    return result;
}

std::ostream& operator<<(std::ostream& os, const std::vector<int>& vec) {
    os << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        os << vec[i];
        if (i != vec.size() - 1)
            os << ", ";
    }
    os << "]";
    return os;
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

bool isBetter(Game& g,int v1,int v2) {
    if ((g.reward==MIN && v1 < v2) || (g.reward==MAX && v1 > v2)) {
        return true;
    }
    return false;
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
splay getPlayMemo(Game& g, std::vector<int> path, int v, std::vector<splay>& memo) 
{
    int index = findVertex(v,path);
    if (index >= 0) {
        int best = bestcolor(g,index,path);
        return {v, best, g.colors[v]};
    }
    else {
        int p = g.owners[v];
        splay next;
        for(auto& e : g.outs[v]) {
            int w = g.targets[e];

            if (memo[e].touched() && findVertex(memo[e].loop,path)>=0) {
                next = memo[e];

                int index = findVertex(memo[e].loop,path);
                int until = bestcolor(g,index,path);
                if ( isBetter(g, until, next.from) ) {
                    next.best = until;
                }
                else {
                    next.best = next.from;
                }
                if (next.parity() != memo[e].parity()) {
                    next = getPlayMemo(g, path+v, w, memo);
                }
            }
            else {
                next = getPlayMemo(g, path+v, w, memo);
            }

            if (next.touched()) {

                if ( isBetter(g, g.colors[v], next.from)) {
                    next.from = g.colors[v];
                }
                memo[e] = next;
            }

            if (v == next.loop && g.owners[v]!=next.parity()) {
                next.loop = -1;
            }

            if (next.parity() == p) 
                return next;
        }
        return next;
    }
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

    return play.parity();
}


//-----------------------------------------------------------------------------------------------------

bool getAllCycles(Game& g, std::vector<int> path, int v, std::vector<bool>& touched) {
    int index = findVertex(v,path);
    if (index >= 0) {
        int best = bestcolor(g,index,path);
        std::cout << path+v << std::endl;
        return best%2;
    }
    else {
        int p = g.owners[v];
        for(auto& e : g.outs[v]) {
            if (touched[v]) {
                std::cout<< path+v << " *" << std::endl;
                continue;
            }
            auto next = getAllCycles(g, path+v, g.targets[e], touched);
            // if (next == p) {
            //     return p;
            // }
        }
        touched[v] = true;
        return 1-p;
    }
}