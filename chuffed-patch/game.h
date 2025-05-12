#ifndef GAME_H
#define GAME_H

#include <iostream>
#include <vector>
#include <fstream>
#include <regex>
#include <sstream>
#include <cassert>
#include <algorithm>
#include <random>
#include <chrono> 

enum parity_type    {EVEN,ODD};                 // 0,1
enum reward_type    {MIN,MAX};                  // 0,1
enum game_type      {DEF,JURD,DZN,GM,RAND};     // 0,1

//======================================================================================

class Game {
public:
    friend class SATEncoder;
    friend class CPModel;
    friend std::vector<int> attractor(std::vector<int>& V, int q, Game& g);
    friend int main(int, char*[]);
public:
    std::vector<int>    owners;
    std::vector<int>    colors;
    std::vector<int>    sources;
    std::vector<int>    targets;
    std::vector<std::vector<int>>   outs;
    std::vector<std::vector<int>>   ins;
    int nvertices;
    int nedges;
    int start;
    reward_type reward;

    //----------------------------------------------------------------------------------

    void fixStartingZero();
    void parseline_dzn(const std::string& line,std::vector<int>& myvec);
    void parseline_gm(  const std::string& line,std::vector<int>& vinfo, 
                        std::vector<int>& outs, std::string& comment);

    //----------------------------------------------------------------------------------

public:

    Game(   std::vector<int> own,std::vector<int> col,
            std::vector<int> sou,std::vector<int> tar, 
            int startv, reward_type rew=MIN);
    Game(int type, std::string filename, int start, reward_type rew=MIN);
    Game(int type, std::vector<int> vals, int start, reward_type rew=MIN);

    void setStart(int startv);
    void setReward(reward_type rew);
    void exportFile(int type, std::string filename);
    void printGame();

    //----------------------------------------------------------------------------------
};

#endif // GAME_H