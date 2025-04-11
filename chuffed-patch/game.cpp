#ifndef game_cpp
#define game_cpp

#include <iostream>
#include <vector>
#include <fstream>
#include <regex>
#include <sstream>

const int EVEN  = 0;
const int ODD   = 1;

//======================================================================================

class Game {
public:
    static const int DZN    = 0;
    static const int GM     = 1;

    friend class SATEncoder;
    friend class CPModel;
    friend std::vector<int> attractor(std::vector<int>& V, int q, Game& g);
    friend int main(int, char*[]);
public:
    std::vector<int>    owners;
    std::vector<int>    colors;
    std::vector<int>    sources;
    std::vector<int>    targets;
    std::vector<std::vector<int>>    edges;
    int nvertices;
    int nedges;
    int start;

    //----------------------------------------------------------------------------------

    void fixStartingZero() {
        for (int i=0; i<sources.size(); i++) {
            sources[i]--;
            targets[i]--;
        }
    }

    //----------------------------------------------------------------------------------

    void parseline_dzn(const std::string& line,std::vector<int>& myvec) {
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

    void parseline_gm(  const std::string& line,std::vector<int>& vinfo, 
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

public:

    Game(   std::vector<int> own,std::vector<int> col,
            std::vector<int> sou,std::vector<int> tar, int startv) 
    :   owners(own), colors(col), sources(sou), targets(tar), start(startv) 
    {
        nvertices   = own.size();
        nedges      = sou.size();

        edges.resize(nvertices);

        for(int i=0; i<nvertices; i++) {
            owners[i]=own[i];
            colors[i]=col[i];
        }
        for(int i=0; i<nedges; i++) {
            sources[i]=sou[i];
            targets[i]=tar[i];
            edges[sources[i]].push_back(i);
        }
    }

    //----------------------------------------------------------------------------------

    Game(std::string filename, int start, int type=DZN) 
    : nvertices(0), nedges(0), start(start) 
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
                fixStartingZero();
                edges.resize(nvertices);
                for(int i=0; i<nedges; i++) {
                    edges[sources[i]].push_back(i);
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
                        verts.reserve(lastvertex+1);
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
                edges.resize(nvertices);

                nedges = 0;
                for(int s=0; s<nvertices; s++) {
                    for(int t=1; t<tedges[s].size(); t++) {
                        sources.push_back(verts[tedges[s][0]]);
                        targets.push_back(verts[tedges[s][t]]);
                        edges[verts[tedges[s][0]]].push_back(nedges);
                        nedges++;
                    }
                }

                // nedges = sources.size();

                break;
            }
        }
    }

    //----------------------------------------------------------------------------------

    Game( int levels, int blocks, int start ) : start(start) {
        nvertices   = ((blocks*3)+1)*(levels-1) + ((blocks*2)+1);
        nedges      = (blocks*6)*(levels-1) + (blocks*4) + (blocks*2*(levels-1));

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
        edges.resize(nvertices);
        for(int i=0; i<nedges; i++) {
            edges[sources[i]].push_back(i);
        }
    }

    //----------------------------------------------------------------------------------

    void printGame() {
        std::cout << "nvertices: " << owners.size() << std::endl;
        std::cout << "owners:    {";
        for(int v=0; v<owners.size(); v++) 
            std::cout<<owners[v]<<(v<owners.size()-1?",":"");
        std::cout << "}" << std::endl;
        std::cout << "colors:    {";
        for(int v=0; v<colors.size(); v++) 
            std::cout<<colors[v]<<(v<colors.size()-1?",":"");
        std::cout << "}" << std::endl;

        std::cout << "nedges:    " << sources.size() << std::endl;
        std::cout << "sources:   {";
        for(int e=0; e<sources.size(); e++) 
            std::cout<<sources[e]<<(e<sources.size()-1?",":"");
        std::cout << "}" << std::endl;
        std::cout << "targets:   {";
        for(int e=0; e<targets.size(); e++) 
            std::cout<<targets[e]<<(e<targets.size()-1?",":"");
        std::cout << "}" << std::endl;
        std::cout << "start:     " << start << std::endl;
    }
};

#endif // game_cpp