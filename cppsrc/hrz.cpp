#include <iostream>
#include <vector>
#include <initializer_list>
#include <cstring>

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

std::pair<int, std::vector<int>> getPlay(Game& g, int p, std::vector<int> path, int current) {

    auto mincolor = [&g](int index,std::vector<int>& path) -> int {
        int min = g.colors[path[index]];
        for (int i=index+1; i<path.size(); i++) {
            if (g.colors[path[i]] < min) {
                min = g.colors[path[i]];
            }
        }
        return min;
    };

    int index = findVertex(current,path);
    if (index >= 0) {
        int best = mincolor(index,path);
        return {best%2, path};
    }
    else {
        if (g.owners[current] == p) {
            std::pair<int, std::vector<int>> detour;
            for(auto& e : g.edges[current]) {
                std::vector <int> newpath = path;
                newpath.push_back(current);
                detour = getPlay(g, p, newpath, g.targets[e]);
                if (detour.first == p) {
                    return detour;
                }
            }
            return detour;
        }
        else {
            return getPlay(g, 1-p , path, current);
        }
    }
}

struct options {
    int game_print      = 0; 
    int game_type       = 0; // 0=default 1=jurd 2=dzn 3=gm
    int jurd_levels     = 2;
    int jurd_blocks     = 1;
    int game_start      = 0;
    std::string game_filename   = "";
} options;

//======================================================================================

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
            if (errno == ERANGE || levels < 2 || levels > 1000) {
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
            if (errno == ERANGE || blocks < 1 || blocks > 1000) {
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
        else if (strcmp(argv[i],"-gm")==0) {
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
            char* endptr;
            int start = std::strtol(argv[i],&endptr,10);
            if (errno == ERANGE || start < 0 || start > 1000000) {
                std::cerr << "ERROR: Starting vertex out of range\n";
                return false;
            }
            if (*endptr != '\0') {
                std::cerr << "ERROR: Starting vertex no numeric\n";
                return false;
            }
            options.game_start = start;
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
        else if (strcmp(argv[i],"--help")==0) {
            std::cout << "Usage: " << argv[0] << " [options]\n";
            std::cout << "Options:\n";
            std::cout << "  --jurd <levels> <blocks>  : Jurdzinski game with <levels> levels and <blocks> blocks\n";
            std::cout << "  --dzn <filename>          : DZN file name\n";
            std::cout << "  --gm <filename>           : GM file name\n";
            std::cout << "  --start <vertex>          : Starting vertex\n";
            std::cout << "  --print <type>            : Print game (0=Parity, 1=Path+Parity, 2=verbose)\n";
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

    switch (options.game_type) {
        case 1: // jurd
            game = new Game(options.jurd_levels, options.jurd_blocks, options.game_start);
            break;
        case 2: // dzn
            game = new Game(options.game_filename, options.game_start, Game::DZN);
            break;
        case 3: // gm
            game = new Game(options.game_filename, options.game_start, Game::GM);
            break;
        default:
            game = new Game({0,1},{3,2},{0,1},{1,0},0);        
            break;
    }

    std::vector<int> path;
    auto play = getPlay(*game, EVEN, path, options.game_start);

    switch (options.game_print) {
        case 0:
            std::cout << (play.first==EVEN?"EVEN":"ODD") << std::endl;
            break;
        case 1:
            std::cout   << play.second
                        << " " << (play.first==EVEN?"EVEN":"ODD") << std::endl; 
            break;
        case 2:
            game->printGame();
            std::cout   << play.second
                        << " " << (play.first==EVEN?"EVEN":"ODD") << std::endl; 
            break;    
        default:
            break;
    }

    delete game;

    return 0;
}


