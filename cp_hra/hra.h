#ifndef HRA_H
#define HRA_H

#include <iostream>
#include <vector>
#include <initializer_list>
#include <cstring>
#include <chrono>

#ifndef game_h 
#include "../chuffed-patch/game.h"
#endif

int findVertex(int vertex,std::vector<int>& path);
int bestcolor(Game& g, int index,std::vector<int>& path);
signed char getPlay(Game& g, int p, std::vector<int> path, int current);
signed char getPlayMemo(Game& g, int p, std::vector<int> path, int current, std::vector<int>& memo);

#endif // HRA_H