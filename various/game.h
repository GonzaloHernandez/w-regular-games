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
#include <climits>

enum parity_type    {EVEN,ODD};                         // 0,1
enum reward_type    {MIN,MAX};                          // 0,1
enum game_type      {DEF,JURD,RAND,MLADDER,DZN,GM,DIM}; // 0,1,2,3,4,5
enum parity_comp    {BET,EQU,BEQ};

//--------------------------------------------------------------------------------------

parity_type opponent(parity_type PARITY);

//======================================================================================

class Game {
public:
    friend class SATEncoder;
    friend class CPModel;
    friend std::vector<int> attractor(std::vector<int>& V, int q, Game& g);
    friend int main(int, char*[]);
public:
    std::vector<int>        owners;
    std::vector<long long>  colors;
    std::vector<int>        sources;
    std::vector<int>        targets;
    std::vector<std::vector<int>>   outs;
    std::vector<std::vector<int>>   ins;
    int nvertices;
    int nedges;
    int start;
    reward_type reward;

    //----------------------------------------------------------------------------------

    void fixStartingZero();
    void parseline_dzn  (const std::string& line,std::vector<int>& myvec);
    void parseline_dzn  (const std::string& line,std::vector<long long>& myvec);
    void parseline_gm   (const std::string& line,std::vector<long long>& vinfo, 
                        std::vector<int>& outs, std::string& comment);

    //----------------------------------------------------------------------------------

public:
    
    Game(   std::vector<int> own,std::vector<long long> col,
            std::vector<int> sou,std::vector<int> tar, 
            int startv, reward_type rew=MIN);
    Game(int type, std::string filename, int start, reward_type rew=MIN);
    Game(int type, std::vector<int> vals, int start, reward_type rew=MIN);

    void setStart(int startv);
    void setReward(reward_type rew);
    bool compareVertices(int v1,int v2,parity_comp rel=BET);
    bool compareColors(int c1,int c2,parity_comp rel=BET);
    void exportFile(int type, std::string filename);
    void printGame();
    void flipGame();
};

// =======================================================================================

class GameView {
private:
    Game& g;
public:
    std::unique_ptr<bool[]> vs;
    std::unique_ptr<bool[]> es;
    GameView(Game& g);

    //----------------------------------------------------------------------------------

    std::vector<int> getVertices();     // Only return a set of active vertices
    std::vector<int> getEdges();        // Only return a set of active edges
    std::vector<int> getOuts(int v);    // Only return a set of active outs of v
    std::vector<int> getIns(int w);     // Only return a set of active ins of w
    std::string viewCurrent();

    void activeAll();
    void deactiveAll();
};

#endif // GAME_H