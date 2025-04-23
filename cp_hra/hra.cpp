#include <iostream>
#include <vector>
#include <initializer_list>
#include <cstring>
#include <chrono>

#ifndef game_cpp 
#include "../chuffed-patch/game.cpp"
#endif

inline std::ostream& operator<<(std::ostream& os, const std::vector<int>& obj) {
    os << "{";
    for (int i = 0; i < obj.size(); i++) {
        os << obj[i];
        if (i<obj.size()-1) os << ",";
    }
    os << "}";
    return os;
}

inline std::string wVectorInt(std::vector<int>& data) {
    std::stringstream ss;
    ss << data;
    return ss.str();
}

inline void launchdebugwatchs() {
    std::vector<int> svi;
    wVectorInt(svi);

}

int findVertex(int vertex,std::vector<int>& path) {
    for (int i=0; i<path.size(); i++) {
        if (path[i] == vertex) return i;
    }
    return -1;
}

int bestcolor(Game& g, int index,std::vector<int>& path){
    int m = g.colors[path[index]];
    for (int i=index+1; i<path.size(); i++) {
        if (g.reward==MIN && g.colors[path[i]] < m) {
            m = g.colors[path[i]];
        }
        else if (g.reward==MAX && g.colors[path[i]] > m) {
            m = g.colors[path[i]];
        }
    }
    return m;
}

signed char getPlay(Game& g, int p, std::vector<int> path, int current) {
    int index = findVertex(current,path);
    if (index >= 0) {
        int best = bestcolor(g,index,path);
        return best%2;
    }
    else {
        if (g.owners[current] == p) {
            for(auto& e : g.vedges[current]) {
                std::vector <int> newpath = path;
                newpath.push_back(current);
                auto next = getPlay(g, p, newpath, g.targets[e]);
                if (next == p) {
                    return p;
                }
            }
            return 1-p;
        }
        else {
            return getPlay(g, 1-p , path, current);
        }
    }
}

// signed char getPlayMemo(Game& g, int p, std::vector<int> path, int current, std::vector<int>& memo) {
//     int index = findVertex(current,path);
//     if (index >= 0) {
//         int best = bestcolor(g,index,path);
//         return best%2;
//     }
//     else {
//         if (g.owners[current] == p) {
//             for(auto& e : g.vedges[current]) {
//                 if (memo[g.targets[e]] == p) {
//                     memo[current] = p;
//                     return p; // with this edge current can force p (already memoized)
//                 }

//                 if (memo[g.targets[e]] == 1-p) {
//                     continue; // skip this edge
//                 }

//                 std::vector <int> newpath = path;
//                 newpath.push_back(current);
//                 auto next = getPlayMemo(g, p, newpath, g.targets[e], memo);
//                 if (next == p) {
//                     memo[current] = p;
//                     return p; // with this edge current can force p
//                 }
//             }
//             memo[current] = 1-p;
//             return 1-p;
//         }
//         else {
//             return getPlayMemo(g, 1-p , path, current, memo);
//         }
//     }
// }

signed char getPlayMemo(Game& g, int p, std::vector<int> path, int current, std::vector<int>& memo) {
    int index = findVertex(current,path);
    if (index >= 0) {
        int best = bestcolor(g,index,path);
        return best%2;
    }
    else {
        int np = ( g.owners[current]==p ? p : 1-p );
        for(auto& e : g.vedges[current]) {
            if (memo[g.targets[e]] == np) {
                memo[current] = np;
                return np; // using this edge current can force np (already memoized)
            }

            if (memo[g.targets[e]] == 1-np) {
                continue; // skip this edge
            }

            std::vector <int> newpath = path;
            newpath.push_back(current);
            auto next = getPlayMemo(g, np, newpath, g.targets[e], memo);
            if (next == np) {
                memo[current] = np;
                return np; // using this edge current can force np 
            }
        }
        memo[current] = 1-np;
        return 1-np;
    }
}

//======================================================================================

struct options {
    int game_print      = 0; 
    int game_type       = 0; // 0=default 1=jurd 2=dzn 3=gm
    int jurd_levels     = 2;
    int jurd_blocks     = 1;
    int game_parity     = 0; // looking for 0=EVEN or 1=ODD
    reward_type game_reward         = MIN; 
    std::vector<int> game_starts    = {0};
    std::string game_filename       = "";
} options;


bool parseOptions(int argc, char *argv[]) {
    for (int i=1; i<argc; i++) {
        if (strcmp(argv[i],"--jurd")==0) {
            options.game_type = 1;
            i++;
            
            if (i>=argc || argv[i][0] == '-') {
                std::cerr << "ERROR: Number of levels missing\n";
                return false;                    
            }
            char* endptr;
            int levels = std::strtol(argv[i],&endptr,10);
            if (errno == ERANGE || levels < 2 || levels > 1000000) {
                std::cerr << "ERROR: Jurdzinski level out of range\n";
                return false;
            }
            if (*endptr != '\0') {
                std::cerr << "ERROR: Jurdzinski level no numeric\n";
                return false;
            }
            options.jurd_levels = levels;

            i++;
            if (i>=argc || argv[i][0] == '-') {
                std::cerr << "ERROR: Number of blocks missing\n";
                return false;                    
            }
            int blocks = std::strtol(argv[i],&endptr,10);
            if (errno == ERANGE || blocks < 1 || blocks > 1000000) {
                std::cerr << "ERROR: Jurdzinski blocks out of range\n";
                return false;
            }            
            if (*endptr != '\0') {
                std::cerr << "ERROR: Jurdzinski blocks no numeric\n";
                return false;
            }
            options.jurd_blocks = blocks;
        }
        else if (strcmp(argv[i],"--dzn")==0) {
            options.game_type = 2;
            i++;
            if (i>=argc || argv[i][0] == '-') {
                std::cerr << "ERROR: DZN file name missing\n";
                return false;                    
            }
            options.game_filename = argv[i];                
        }
        else if (strcmp(argv[i],"--gm")==0) {
            options.game_type = 3;
            i++;
            if (i>=argc || argv[i][0] == '-') {
                std::cerr << "ERROR: GM file name missing\n";
                return false;                    
            }
            options.game_filename = argv[i];                
        }
        else if (strcmp(argv[i],"--start")==0) {
            i++;
            if (i>=argc || argv[i][0] == '-') {
                std::cerr << "ERROR: Starting vertex missing\n";
                return false;                    
            }

            options.game_starts.clear();
            std::string s = argv[i];
            std::stringstream ss(s);
            std::string item;
            while (std::getline(ss, item, ',')) {
                size_t start = item.find_first_not_of(" \t");
                size_t end = item.find_last_not_of(" \t");

                if (start == std::string::npos || end == std::string::npos) {
                    std::cerr << "ERROR: Empty or invalid element found between commas\n";
                    return false;
                }

                options.game_starts.push_back(std::stoi(item));
            }
        }
        else if (strcmp(argv[i],"--print")==0) {
            i++;
            if (i>=argc || argv[i][0] == '-') {
                std::cerr << "ERROR: Print type number missing\n";
                return false;
            }

            char* endptr;
            int print = std::strtol(argv[i],&endptr,10);
            if (errno == ERANGE || print < 0 || print > 2) {
                std::cerr << "ERROR: Print type number out range (0,1,2)\n";
                return false;
            }
            if (*endptr != '\0') {
                std::cerr << "ERROR: Print type no numeric\n";
                return false;
            }
            options.game_print = print;
        }
        else if (strcmp(argv[i],"--parity-even")==0) {
            options.game_parity = EVEN;
        }
        else if (strcmp(argv[i],"--parity-odd")==0) {
            options.game_parity = ODD;
        }
        else if (strcmp(argv[i],"--reward-max")==0) {
            options.game_reward = MAX;
        }
        else if (strcmp(argv[i],"--reward-min")==0) {
            options.game_reward = MIN;
        }
        else if (strcmp(argv[i],"--help")==0) {
            std::cout << "Usage: " << argv[0] << " [options]\n";
            std::cout << "Options:\n";
            std::cout << "  --jurd <levels> <blocks>  : Jurdzinski game with <levels> levels and <blocks> blocks\n";
            std::cout << "  --dzn <filename>          : DZN file name\n";
            std::cout << "  --gm <filename>           : GM file name\n";
            std::cout << "  --start <vertex>          : Starting vertex\n";
            std::cout << "  --print <type>            : Print game (0=Parity, 1=Parity+Time, 2=Verbose)\n";
            std::cout << "  --parity-even             : Search for play EVEN\n";
            std::cout << "  --parity-odd              : Search for play ODD\n";
            std::cout << "  --reward-max              : Seek to maximize the color\n";
            std::cout << "  --reward-min              : Seek to minimize the color\n";
            return false;
        }
        else {
            std::cerr << "ERROR: Unknown option: " << argv[i] << std::endl;
            return false;
        }
    }
    return true;
}

// ------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    launchdebugwatchs();
    if (!parseOptions(argc, argv)) exit(1);
    Game* game = nullptr;

    //------------------------------------------------------------

    auto start = std::chrono::high_resolution_clock::now();

    switch (options.game_type) {
        case 1: // jurd
            game = new Game(options.jurd_levels, options.jurd_blocks, 
                            options.game_starts[0], options.game_reward);
            break;
        case 2: // dzn
            game = new Game(Game::DZN, options.game_filename,
                            options.game_starts[0], options.game_reward);
            break;
        case 3: // gm
            game = new Game(Game::GM, options.game_filename,
                            options.game_starts[0], options.game_reward);
            break;
        default:
            game = new Game({0,1},{3,2},{0,1},{1,0},EVEN,MIN);
            break;
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> preptime = end - start;

    if (options.game_print == 2) {
        game->printGame();
    }

    while (true) {
        std::vector<int> path;
        std::vector<int> memo(game->nvertices, -1);

        start = std::chrono::high_resolution_clock::now();

        auto play = getPlayMemo(*game, options.game_parity, path, options.game_starts[0],memo);

        end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> totaltime = end - start;

        switch (options.game_print) {
            case 0:
                std::cout   << options.game_starts[0] << ": "
                            << (play==EVEN?"EVEN":"ODD") 
                            << std::endl;
                break;
            case 1: case 2:
                std::cout   << options.game_starts[0] << ": "
                            << (play==EVEN?"EVEN":"ODD") << " "
                            << preptime.count() << " " << totaltime.count() 
                            << std::endl; 
                break;
            default:
                break;
        }

        options.game_starts.erase(options.game_starts.begin());
        if (options.game_starts.size() == 0) break;
        game->setStart(options.game_starts[0]);
    }

    delete game;

    return 0;
}


