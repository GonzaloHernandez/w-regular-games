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
                        std::vector<int>& outs, std::string& comment) 
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
            outs.push_back(std::stoi(num));
        }
        comment = matches[5];
    }

}

//----------------------------------------------------------------------------------

void Game::commonConstructor() {
    currentv = std::make_unique<bool[]>(nvertices);
    std::fill_n(currentv.get(), nvertices, true);
    currente = std::make_unique<bool[]>(nedges);
    std::fill_n(currente.get(), nedges, true);
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

    outs.resize(nvertices);
    ins .resize(nvertices);

    for(int i=0; i<nvertices; i++) {
        owners[i]=own[i];
        colors[i]=col[i];
    }
    for(int i=0; i<nedges; i++) {
        sources[i]=sou[i];
        targets[i]=tar[i];
        outs[sources[i]].push_back(i);
        ins [targets[i]].push_back(i);
    }
    commonConstructor();
}

//----------------------------------------------------------------------------------
// Imported game from DZN or GM

Game::Game(int type, std::string filename, int start, reward_type rew) 
:   nvertices(0), nedges(0), start(start), reward(rew) 
{
    if (!filename.empty() && filename.back() == '.') {
        switch (type) {
            case DZN:   filename.append("dzn"); break;
            case GM:    filename.append("gm");  break;
        }
    }
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
            outs.resize(nvertices);
            ins .resize(nvertices);
            for(int i=0; i<nedges; i++) {
                outs[sources[i]].push_back(i);
                ins [targets[i]].push_back(i);
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
                    std::vector<int>    vinfo,outs;
                    std::string         comment;
                    parseline_gm(line,vinfo,outs,comment);
                    verts[vinfo[0]] = counter;
                    outs.insert(outs.begin(),vinfo[0]); // check
                    tedges.push_back(outs);
                    owners.push_back(vinfo[2]);
                    colors.push_back(vinfo[1]);
                    counter++;
                }
            }
            file.close();

            nvertices = counter;

            assert(start >= 0 && start < nvertices);

            outs.resize(nvertices);
            ins .resize(nvertices);

            nedges = 0;
            for(int s=0; s<nvertices; s++) {
                for(int t=1; t<tedges[s].size(); t++) {
                    sources.push_back(verts[tedges[s][0]]);
                    targets.push_back(verts[tedges[s][t]]);
                    outs[verts[tedges[s][0]]].push_back(nedges);
                    ins [verts[tedges[s][t]]].push_back(nedges);
                    nedges++;
                }
            }

            // nedges = sources.size();

            break;
        }
    }
    commonConstructor();
}

//----------------------------------------------------------------------------------
// Jurdzinski game

Game::Game(int type, std::vector<int> vals, int start, reward_type rew) 
:   start(start), reward(rew)  
{
    if (type == JURD) {
        int levels  = vals[0];
        int blocks  = vals[1];    
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
        outs.resize(nvertices);
        ins .resize(nvertices);
        for(int i=0; i<nedges; i++) {
            outs[sources[i]].push_back(i);
            ins [targets[i]].push_back(i);
        }
    }
    else if (type == RAND) {
        nvertices   = vals[0];
        nedges      = 0;
    
        assert(start >= 0 && start < nvertices);
    
        std::random_device rd;
        std::mt19937 g(rd());
    
        owners.resize(nvertices/2,0);
        owners.resize(nvertices,1);
        std::shuffle(owners.begin(), owners.end(), g);  // Unsort (shuffle) the vector
    
        std::uniform_int_distribution<> rndcolors(0, vals[1]);
    
        for(int i=0; i<nvertices; i++) {
            colors.push_back(rndcolors(g));
        }
    
        outs.resize(nvertices);
        ins .resize(nvertices);
        for(int v=0; v<nvertices; v++) {
            std::vector<int> ws;
            for (int i=0; i < nvertices; i++) { ws.push_back(i); }
            std::shuffle(ws.begin(), ws.end(), g);
    
            std::uniform_int_distribution<> rndnedges(vals[2], vals[3]);
            int es = rndnedges(g);
            for (int i=0; i<es; i++) {
                sources.push_back(v);
                targets.push_back(ws[i]);
                outs[v].push_back(nedges);
                ins[ws[i]].push_back(nedges);
                nedges++;
            }
        }
    }
    else if (type == LADDER) {
        int bl = vals[0];
        nvertices   = bl*3+1;
        nedges      = bl*4+1;

        assert(start >= 0 && start < nvertices);

        owners  .resize(nvertices,ODD);
        colors  .resize(nvertices);
        sources .resize(nedges);
        targets .resize(nedges);
        outs    .resize(nvertices);
        ins     .resize(nvertices);

        int consecutive = bl*2;
        colors[0] = consecutive--;
        for (int i=0; i<bl; i++) {
            colors[i*3+1] = 0;
            colors[i*3+2] = consecutive--;
            colors[i*3+3] = consecutive--;
        }

        int e = 0;
        for (int i=0; i<bl; i++) {
            sources[e] = i*3+0;
            targets[e] = i*3+1;
            outs[i*3+0].push_back(e);
            ins [i*3+1].push_back(e);
            e++;

            sources[e] = i*3+1;
            targets[e] = i*3+2;
            outs[i*3+1].push_back(e);
            ins [i*3+2].push_back(e);
            e++;

            sources[e] = i*3+1;
            targets[e] = i*3+3;
            outs[i*3+1].push_back(e);
            ins [i*3+3].push_back(e);
            e++;

            sources[e] = i*3+2;
            targets[e] = i*3+3;
            outs[i*3+2].push_back(e);
            ins [i*3+3].push_back(e);
            e++;
        }

        sources[e] = bl*3;
        targets[e] = 0;
        outs[bl*3].push_back(e);
        ins [0].push_back(e);
    }
    commonConstructor();
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
            for (int e=0; e<outs[v].size(); e++) {
                file << (e?",":"") << targets[outs[v][e]];
            }
            file << ";" << std::endl;
        }
        break;
    }
}

//----------------------------------------------------------------------------------

std::vector<int> Game::getVertices() {
    std::vector<int> vs;
    for (int v=0; v<nvertices; v++) {
        if (currentv[v]) vs.push_back(v);
    }
    return vs;
}

//----------------------------------------------------------------------------------

std::vector<int> Game::getEdges() {
    std::vector<int> es;
    for (int e=0; e<nedges; e++) {
        if (currente[e]) es.push_back(e);
    }
    return es;
}

//----------------------------------------------------------------------------------

void Game::activeAll() {
    for (int v=0; v<nvertices; v++) {
        currentv[v] = true;
    }
    for (int e=0; e<nedges; e++) {
        currente[e] = true;
    }
}

//----------------------------------------------------------------------------------

void Game::deactiveAll() {
    for (int v=0; v<nvertices; v++) {
        currentv[v] = false;
    }
    for (int e=0; e<nedges; e++) {
        currente[e] = false;
    }
}

//----------------------------------------------------------------------------------

std::vector<int> Game::getOuts(int v) {
    std::vector<int> es;
    for(auto& e : outs[v]) {
        int w = targets[e];
        if (currente[e] && currentv[w]) es.push_back(e);
    }
    return es;
}

//----------------------------------------------------------------------------------

std::vector<int> Game::getIns(int w) {
    std::vector<int> es;
    for(auto& e : ins[w]) {
        int v = sources[e];
        if (currente[e] && currentv[v]) es.push_back(e);
    }
    return es;
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

std::string Game::viewCurrent() {
    std::stringstream ss;
    ss << "{";
    for(int i=0; i<nvertices; i++) if (currentv[i]) {
        if (i>0) ss << ",";
        ss << i << ",";
    }
    ss << "} {";
    for(int i=0; i<nedges; i++) if (currente[i]) {
        if (i>0) ss << ",";
        ss << i << ",";
    }
    ss << "}";

    return ss.str();
}
