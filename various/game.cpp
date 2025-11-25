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

parity_type opponent(parity_type PARITY) {
    if (PARITY==EVEN) return ODD; return EVEN;
}

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

void Game::parseline_dzn(const std::string& line,std::vector<long long>& myvec) {
    std::regex pattern(R"(\[(.*?)\])");
    std::smatch match;

    if (regex_search(line, match, pattern)) {
        std::string values = match[1];
        std::stringstream ss(values);
        std::string value;

        while (getline(ss, value, ',')) {
            myvec.push_back(stoll(value));
        }
    }
}

//----------------------------------------------------------------------------------


#include <cstdio>  // For sscanf
#include <cstring> // For strstr (useful for finding comment start)
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>

#include <string>
#include <vector>
#include <algorithm>
#include <cctype>

size_t skip_whitespace(const std::string& line, size_t start) {
    while (start < line.size() && std::isspace(line[start])) {
        start++;
    }
    return start;
}

size_t find_token_end(const std::string& line, size_t start, char delimiter) {
    size_t end = line.find(delimiter, start);
    if (end == std::string::npos) {
        end = line.size();
    }
    size_t non_space_end = start;
    for (size_t i = start; i < end; ++i) {
        if (!std::isspace(line[i])) {
            non_space_end = i + 1;
        }
    }
    return non_space_end;
}

void Game::parseline_gm(const std::string& line, std::vector<long long>& vinfo, 
                        std::vector<int>& outs_targets, std::string& comment) 
{
    vinfo.clear();
    outs_targets.clear();
    comment.clear();

    size_t current = 0;
    size_t next;
    
    // --- 1. Extract Vertex ID, Priority, Owner (Space-separated) ---
    for (int i = 0; i < 3; ++i) {
        current = skip_whitespace(line, current);
        if (current >= line.size()) return; // Malformed line

        next = find_token_end(line, current, ' ');
        if (next == current) return; // Empty token

        vinfo.push_back(std::stoll(line.substr(current, next - current)));
        current = next;
    }
    
    // --- 2. Extract Target Edges (Comma-separated list) ---
    // Move 'current' past the space separator after the Owner
    current = skip_whitespace(line, current);

    size_t end_of_targets_block = line.find_first_of("\" \t;", current);
    if (end_of_targets_block == std::string::npos) {
        end_of_targets_block = line.size();
    }
    
    // Take the whole block that might contain the targets (e.g., "1,2,3")
    std::string targets_block = line.substr(current, end_of_targets_block - current);
    
    size_t target_start = 0;
    size_t target_comma;

    // Use string::find for the comma delimiter inside the targets block
    while ((target_comma = targets_block.find(',', target_start)) != std::string::npos) {
        std::string num_str = targets_block.substr(target_start, target_comma - target_start);
        
        // Trim and convert the number
        size_t first = num_str.find_first_not_of(" \t");
        size_t last = num_str.find_last_not_of(" \t");
        
        if (first != std::string::npos) {
            outs_targets.push_back(std::stoi(num_str.substr(first, last - first + 1)));
        }
        target_start = target_comma + 1;
    }
    
    // Process the last number (or the only number)
    std::string num_str = targets_block.substr(target_start);
    size_t first = num_str.find_first_not_of(" \t");
    if (first != std::string::npos) {
        size_t last = num_str.find_last_not_of(" \t");
        outs_targets.push_back(std::stoi(num_str.substr(first, last - first + 1)));
    }
    
    // Update main position pointer to continue searching for comment
    current += targets_block.size(); 
    
    // --- 3. Extract Optional Comment ---
    current = skip_whitespace(line, current);

    if (current < line.size() && line[current] == '"') {
        current++; // Move past the opening quote
        size_t comment_end = line.find('"', current);
        if (comment_end != std::string::npos) {
            comment = line.substr(current, comment_end - current);
        }
    }
}

//----------------------------------------------------------------------------------
// Default game

Game::Game( std::vector<int> own,std::vector<long long> col,
            std::vector<int> sou,std::vector<int> tar, 
            int startv, reward_type rew) 
:   owners(own), colors(col), sources(sou), targets(tar), 
    start(startv), reward(rew) 
{
    nvertices   = own.size();
    nedges      = sou.size();

    assert(start >= 0 && start < nvertices && "Starting vertex must be within the valid range");

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
}

//----------------------------------------------------------------------------------
// Imported game from DZN or GM

Game::Game(int type, std::string filename, int start, reward_type rew) 
:   nvertices(0), nedges(0), start(start), reward(rew) 
{
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);

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

            assert(start >= 0 && start < nvertices && "Starting vertex must be within the valid range");

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
                } else if (line.find("start") != std::string::npos) {
                    // ignore start line
                } else {
                    std::vector<long long>  vinfo;
                    std::vector<int>        outs;
                    std::string             comment;
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

            assert(start >= 0 && start < nvertices && "Starting vertex must be within the valid range");

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

        assert(start >= 0 && start < nvertices && "Starting vertex must be within the valid range");

        int es = 1;
        int os = 0;
        
        for (int l=1; l<levels; l++) {
            os = ((blocks*3)+1)*(levels-1)+1;
            for (int b=0; b<blocks; b++) {
                owners.push_back(1);
                owners.push_back(0);
                owners.push_back(0);
                colors.push_back((levels-l)*2);
                colors.push_back((levels-l)*2+1);
                colors.push_back((levels-l)*2);

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
            colors.push_back((levels-l)*2);
            es += 1;
        }
        int l = levels;
        for (int b=0; b<blocks; b++) {
            owners.push_back(0);
            owners.push_back(1);

            colors.push_back((levels-l)*2);
            colors.push_back((levels-l)*2+1);

            sources.push_back(es);   targets.push_back(es+1);
            sources.push_back(es+1); targets.push_back(es);
            sources.push_back(es+1); targets.push_back(es+2);
            sources.push_back(es+2); targets.push_back(es+1);
            
            es += 2;
        }
        owners.push_back(0);
        colors.push_back((levels-l)*2);

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
    
        assert(start >= 0 && start < nvertices && "Starting vertex must be within the valid range");
    
        std::random_device rd;
        std::mt19937 g(rd());
    
        owners.resize(nvertices/2,0);
        owners.resize(nvertices,1);
        std::shuffle(owners.begin(), owners.end(), g);  
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
    else if (type == MLADDER) {
        int bl = vals[0];
        nvertices   = bl*3+1;
        nedges      = bl*4+1;

        assert(start >= 0 && start < nvertices && "Starting vertex must be within the valid range");

        // ---------------------------------------

        std::random_device rd;
        std::mt19937 g(rd());
        owners.resize(nvertices/2,0);
        owners.resize(nvertices,1);
        std::shuffle(owners.begin(), owners.end(), g);  // Unsort (shuffle) the vector

        // owners  .resize(nvertices,ODD);

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
}

//----------------------------------------------------------------------------------

void Game::setStart(int startv) {
    assert(startv >= 0 && startv < nvertices && "Starting vertex must be within the valid range");
    start = startv;
}

//----------------------------------------------------------------------------------

void Game::setReward(reward_type rew) {
    reward = rew;
}

//----------------------------------------------------------------------------------

bool Game::compareVertices(int v1,int v2,parity_comp rel) {
    return compareColors(colors[v1],colors[v2]);
}

//----------------------------------------------------------------------------------

bool Game::compareColors(int c1,int c2,parity_comp rel) {
    switch (rel) {
    case BET:
        if (reward==MIN && c1 < c2) return true; 
        if (reward==MAX && c1 > c2) return true;
        break;   
    case EQU:
        if (reward==MIN && c1 == c2) return true; 
        if (reward==MAX && c1 == c2) return true;
        break;   
    case BEQ:
        if (reward==MIN && c1 <= c2) return true; 
        if (reward==MAX && c1 >= c2) return true;
        break;   
    }
    return false;
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
        for(int i=0; i<sources.size(); i++) file<<(i?",":"")<<sources[i]+1; file<<"];"<<std::endl;
        file << "targets   = ["; 
        for(int i=0; i<targets.size(); i++) file<<(i?",":"")<<targets[i]+1; file<<"];"<<std::endl;
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

void Game::flipGame() {
    for(int v=0; v<nvertices; v++) {
        owners[v] = 1-owners[v];
        colors[v]++;
    }
}

//=====================================================================================

GameView::GameView(Game& g) : g(g) {
    vs = std::make_unique<bool[]>(g.nvertices);
    std::fill_n(vs.get(), g.nvertices, true);
    es = std::make_unique<bool[]>(g.nedges);
    std::fill_n(es.get(), g.nedges, true);
}

//----------------------------------------------------------------------------------

std::vector<int> GameView::getVertices() {
    std::vector<int> vertices;
    for (int v=0; v<g.nvertices; v++) {
        if (vs[v]) vertices.push_back(v);
    }
    return vertices;
}

//----------------------------------------------------------------------------------

std::vector<int> GameView::getEdges() {
    std::vector<int> edges;
    for (int e=0; e<g.nedges; e++) {
        if (es[e]) edges.push_back(e);
    }
    return edges;
}

//----------------------------------------------------------------------------------

void GameView::activeAll() {
    for (int v=0; v<g.nvertices; v++) {
        vs[v] = true;
    }
    for (int e=0; e<g.nedges; e++) {
        es[e] = true;
    }
}

//----------------------------------------------------------------------------------

void GameView::deactiveAll() {
    for (int v=0; v<g.nvertices; v++) {
        vs[v] = false;
    }
    for (int e=0; e<g.nedges; e++) {
        es[e] = false;
    }
}

//----------------------------------------------------------------------------------

std::vector<int> GameView::getOuts(int v) {
    std::vector<int> edges;
    for(auto& e : g.outs[v]) {
        int w = g.targets[e];
        if (es[e] && vs[w]) edges.push_back(e);
    }
    return edges;
}

//----------------------------------------------------------------------------------

std::vector<int> GameView::getIns(int w) {
    std::vector<int> edges;
    for(auto& e : g.ins[w]) {
        int v = g.sources[e];
        if (es[e] && vs[v]) edges.push_back(e);
    }
    return edges;
}

//----------------------------------------------------------------------------------

std::string GameView::viewCurrent() {
    std::stringstream ss;
    ss << "{";
    for(int i=0; i<g.nvertices; i++) if (vs[i]) {
        if (i>0) ss << ",";
        ss << i << ",";
    }
    ss << "} {";
    for(int i=0; i<g.nedges; i++) if (es[i]) {
        if (i>0) ss << ",";
        ss << i << ",";
    }
    ss << "}";

    return ss.str();
}
