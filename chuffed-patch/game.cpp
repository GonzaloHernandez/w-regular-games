#include <iostream>
#include <vector>
#include <fstream>
#include <regex>
#include <sstream>
#include <cassert>
#include <algorithm>
#include <random>
#include <chrono> 
#include "game.h"

//----------------------------------------------------------------------------------

void Game::fixStartingZero() {
    for (int i=0; i<sources.size(); i++) {
        sources[i]--;
        targets[i]--;
    }
}

//----------------------------------------------------------------------------------

void Game::parseline_dzn(const std::string& line,std::vector<int>& myvec) {
    std::regex pattern(R"(\[(.*?)\])");
    std::smatch match;

    if (regex_search(line, match, pattern)) {
        std::string values = match[1];
        std::stringstream ss(values);
        std::string value;

        while (getline(ss, value, ',')) {
            myvec.push_back(stoi(value));
        }
    }
}

//----------------------------------------------------------------------------------

void Game::parseline_gm(const std::string& line,std::vector<int>& vinfo, 
                        std::vector<int>& vedges, std::string& comment) 
{
    std::regex pattern(R"((\d+)\s+(\d+)\s+(\d+)\s+([\d,]+)(?:\s+\"([^"]+)\")?;?)");
    std::smatch matches;

    std::sregex_iterator it(line.begin(), line.end(), pattern);
    std::sregex_iterator end;

    if (std::regex_match(line, matches, pattern)) {
        vinfo.push_back(std::stoi(matches[1]));
        vinfo.push_back(std::stoi(matches[2]));
        vinfo.push_back(std::stoi(matches[3]));

        std::stringstream ss(matches[4]);
        std::string num;
        while (std::getline(ss, num, ',')) {
            vedges.push_back(std::stoi(num));
        }
        comment = matches[5];
    }

}

//----------------------------------------------------------------------------------
// Default game

Game::Game( std::vector<int> own,std::vector<int> col,
            std::vector<int> sou,std::vector<int> tar, 
            int startv, reward_type rew) 
:   owners(own), colors(col), sources(sou), targets(tar), 
    start(startv), reward(rew) 
{
    nvertices   = own.size();
    nedges      = sou.size();

    assert(start >= 0 && start < nvertices);

    vedges.resize(nvertices);

    for(int i=0; i<nvertices; i++) {
        owners[i]=own[i];
        colors[i]=col[i];
    }
    for(int i=0; i<nedges; i++) {
        sources[i]=sou[i];
        targets[i]=tar[i];
        vedges[sources[i]].push_back(i);
    }
}

//----------------------------------------------------------------------------------
// Imported game from DZN or GM

Game::Game(int type, std::string filename, int start, reward_type rew) 
:   nvertices(0), nedges(0), start(start), reward(rew) 
{
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Error: Could not open file!" << std::endl;
        exit(0);
    }

    std::string line;

    switch (type) {
        case DZN:
            while (getline(file, line)) {
                if (line.find("nvertices") != std::string::npos) {
                    nvertices = stoi(line.substr(line.find("=") + 1));
                } else if (line.find("nedges") != std::string::npos) {
                    nedges = stoi(line.substr(line.find("=") + 1));
                } else if (line.find("owners") != std::string::npos) {
                    parseline_dzn(line,owners);
                } else if (line.find("colors") != std::string::npos) {
                    parseline_dzn(line,colors);
                } else if (line.find("sources") != std::string::npos) {
                    parseline_dzn(line,sources);
                } else if (line.find("targets") != std::string::npos) {
                    parseline_dzn(line,targets);
                }
            }
            file.close();

            assert(start >= 0 && start < nvertices);

            fixStartingZero();
            vedges.resize(nvertices);
            for(int i=0; i<nedges; i++) {
                vedges[sources[i]].push_back(i);
            }
            break;

        case GM: {
            int lastvertex = 0;
            std::vector<int> verts;
            std::vector<std::vector<int>> tedges;
            int counter=0;
            while (getline(file, line)) {
                if (line.find("parity") != std::string::npos) {
                    lastvertex = stoi(line.substr(line.find(" ")));
                    verts.resize(lastvertex+1);
                } else {
                    std::vector<int>    vinfo,vedges;
                    std::string         comment;
                    parseline_gm(line,vinfo,vedges,comment);
                    verts[vinfo[0]] = counter;
                    vedges.insert(vedges.begin(),vinfo[0]);
                    tedges.push_back(vedges);
                    owners.push_back(vinfo[2]);
                    colors.push_back(vinfo[1]);
                    counter++;
                }
            }
            file.close();

            nvertices = counter;

            assert(start >= 0 && start < nvertices);

            vedges.resize(nvertices);

            nedges = 0;
            for(int s=0; s<nvertices; s++) {
                for(int t=1; t<tedges[s].size(); t++) {
                    sources.push_back(verts[tedges[s][0]]);
                    targets.push_back(verts[tedges[s][t]]);
                    vedges[verts[tedges[s][0]]].push_back(nedges);
                    nedges++;
                }
            }

            // nedges = sources.size();

            break;
        }
    }
}

//----------------------------------------------------------------------------------
// Jurdzinski game

Game::Game(int levels, int blocks, int start, reward_type rew) 
:   start(start), reward(rew)  
{
    nvertices   = ((blocks*3)+1)*(levels-1) + ((blocks*2)+1);
    nedges      = (blocks*6)*(levels-1) + (blocks*4) + (blocks*2*(levels-1));

    assert(start >= 0 && start < nvertices);

    int es = 1;
    int os = 0;
    for (int l=1; l<levels; l++) {
        os = ((blocks*3)+1)*(levels-1)+1;
        for (int b=0; b<blocks; b++) {
            owners.push_back(1);
            owners.push_back(0);
            owners.push_back(0);
            colors.push_back(l*2);
            colors.push_back(l*2-1);
            colors.push_back(l*2);

            sources.push_back(es);   targets.push_back(es+1);
            sources.push_back(es);   targets.push_back(es+2);
            sources.push_back(es+1); targets.push_back(es+2);
            sources.push_back(es+2); targets.push_back(es);

            sources.push_back(es+2); targets.push_back(es+3);
            sources.push_back(es+3); targets.push_back(es+2);

            sources.push_back(es+2); targets.push_back(os+1);
            sources.push_back(os+1); targets.push_back(es+2);

            es += 3;
            os += 2;
        }
        owners.push_back(1);
        colors.push_back(l*2);
        es += 1;
    }
    int l = levels;
    for (int b=0; b<blocks; b++) {
        owners.push_back(0);
        owners.push_back(1);

        colors.push_back(l*2);
        colors.push_back(l*2-1);

        sources.push_back(es);   targets.push_back(es+1);
        sources.push_back(es+1); targets.push_back(es);
        sources.push_back(es+1); targets.push_back(es+2);
        sources.push_back(es+2); targets.push_back(es+1);
        
        es += 2;
    }
    owners.push_back(0);
    colors.push_back(l*2);

    fixStartingZero();
    vedges.resize(nvertices);
    for(int i=0; i<nedges; i++) {
        vedges[sources[i]].push_back(i);
    }
}

//----------------------------------------------------------------------------------
// Random game

Game::Game(int ns, int ps, int d1, int d2, int start, reward_type rew) 
:   start(start), reward(rew)  
{
    nvertices   = ns;
    nedges      = 0;

    assert(start >= 0 && start < nvertices);

    std::random_device rd;
    std::mt19937 g(rd());

    owners.resize(ns/2,0);
    owners.resize(ns,1);
    std::shuffle(owners.begin(), owners.end(), g);  // Unsort (shuffle) the vector

    std::uniform_int_distribution<> rndcolors(0, ps);

    for(int i=0; i<nvertices; i++) {
        colors.push_back(rndcolors(g));
    }

    vedges.resize(nvertices);
    for(int v=0; v<nvertices; v++) {
        std::vector<int> ws;
        for (int i=0; i < nvertices; i++) { ws.push_back(i); }
        std::shuffle(ws.begin(), ws.end(), g);

        std::uniform_int_distribution<> rndnedges(d1, d2);
        int es = rndnedges(g);
        for (int i=0; i<es; i++) {
            sources.push_back(v);
            targets.push_back(ws[i]);
            vedges[v].push_back(nedges);
            nedges++;
        }
    }
}

//----------------------------------------------------------------------------------

void Game::setStart(int startv) {
    assert(startv >= 0 && startv < nvertices);
    start = startv;
}

//----------------------------------------------------------------------------------

void Game::setReward(reward_type rew) {
    reward = rew;
}

//----------------------------------------------------------------------------------

void Game::exportFile(int type, std::string filename) {
    std::ofstream file(filename);
    if (!file) {
        std::cerr << "Error: Could not open file!" << std::endl;
        exit(0);
    }

    switch (type) {
    case DZN:
        file << "nvertices = " << nvertices << ";" << std::endl;
        file << "owners    = ["; 
        for(int i=0; i<owners.size(); i++)  file<<(i?",":"")<<owners[i];  file<<"];"<<std::endl;
        file << "colors    = ["; 
        for(int i=0; i<colors.size(); i++)  file<<(i?",":"")<<colors[i];  file<<"];"<<std::endl;

        file << "nedges    = " << nedges << ";" << std::endl;
        file << "sources   = ["; 
        for(int i=0; i<sources.size(); i++) file<<(i?",":"")<<sources[i]; file<<"];"<<std::endl;
        file << "targets   = ["; 
        for(int i=0; i<targets.size(); i++) file<<(i?",":"")<<targets[i]; file<<"];"<<std::endl;
        break;

    case GM:
        file << "parity " << nvertices << ";" << std::endl;
        for (int v=0; v<nvertices; v++) {
            file << v << " " << colors[v] << " " << owners[v] << " ";
            for (int e=0; e<vedges[v].size(); e++) {
                file << (e?",":"") << targets[vedges[v][e]];
            }
            file << ";" << std::endl;
        }
        break;
    }
}

//----------------------------------------------------------------------------------

void Game::printGame() {
    std::cout << "nvertices: " << owners.size() << std::endl;
    std::cout << "owners:    {";
    for(int v=0; v<nvertices; v++) 
        std::cout<<owners[v]<<(v<owners.size()-1?",":"");
    std::cout << "}" << std::endl;
    std::cout << "colors:    {";
    for(int v=0; v<nvertices; v++) 
        std::cout<<colors[v]<<(v<colors.size()-1?",":"");
    std::cout << "}" << std::endl;

    std::cout << "nedges:    " << sources.size() << std::endl;
    std::cout << "sources:   {";
    for(int e=0; e<nedges; e++) 
        std::cout<<sources[e]<<(e<sources.size()-1?",":"");
    std::cout << "}" << std::endl;
    std::cout << "targets:   {";
    for(int e=0; e<nedges; e++) 
        std::cout<<targets[e]<<(e<targets.size()-1?",":"");
    std::cout << "}" << std::endl;
    // std::cout << "start:     " << start << std::endl;
}

//----------------------------------------------------------------------------------
