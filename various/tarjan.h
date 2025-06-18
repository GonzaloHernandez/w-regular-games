#ifndef TARJAN_H
#define TARJAN_H

#ifndef game_h
#include "../chuffed-patch/game.h"
#endif

#include <vector>

class TarjanSCC {
private:
    Game& g;
    std::vector<int>    indices;
    std::vector<int>    lowlink;
    std::vector<bool>   onstack;
    std::stack<int>     stack;
    std::vector<std::vector<int>>   sccs;
    int index = 0;
public:
    TarjanSCC(Game& g);
    std::vector<std::vector<int>> solveRAW();
    void searchRAW(int v);

    std::vector<std::vector<int>> solve();
    void search(int v);

    //----------------------------------------------------------------------------------
};

#endif