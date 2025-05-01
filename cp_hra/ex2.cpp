
#include "hra.h"
#include "graph.h"
#include "debugstd.h"

//--------------------------------------------------------------------------------------

struct options {
    int game_print      = 0; 
    int game_type       = 0; // enum game_type{DEF,JURD,DZN,GM,RAND}
    int jurd_levels     = 2;
    int jurd_blocks     = 1;
    int rand_ns         = 2;
    int rand_ps         = 2;
    int rand_d1         = 2;
    int rand_d2         = 2;
    int game_parity     = 0; // looking for 0=EVEN or 1=ODD
    reward_type         game_reward             = MIN; 
    std::vector<int>    game_starts             = {0};
    std::string         game_filename           = "";
    std::string         game_export_filename    = "";
    int                 game_export_type        = 0; // 0=not export 2=DZN 3=GM
    int                 game_proof              = 0; // 0=no 1=yes 2=matrixbased
} options;

//--------------------------------------------------------------------------------------

bool parseOptions(int argc, char *argv[]) {
    for (int i=1; i<argc; i++) {
        if (strcmp(argv[i],"--jurd")==0) {
            options.game_type = JURD;
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
        else if (strcmp(argv[i],"--rand")==0) {
            options.game_type = RAND;
            i++;
            
            if (i>=argc || argv[i][0] == '-') {
                std::cerr << "ERROR: Number of vertices missing\n";
                return false;                    
            }
            char* endptr;
            int ns = std::strtol(argv[i],&endptr,10);
            if (errno == ERANGE || ns < 1 || ns > 10000000) {
                std::cerr << "ERROR: Number of vertices out of range\n";
                return false;
            }
            if (*endptr != '\0') {
                std::cerr << "ERROR: Number of vertices no numeric\n";
                return false;
            }
            options.rand_ns = ns;

            i++;
            if (i>=argc || argv[i][0] == '-') {
                std::cerr << "ERROR: Number of priorities missing\n";
                return false;                    
            }
            int ps = std::strtol(argv[i],&endptr,10);
            if (errno == ERANGE || ps < 1 || ps > 10000000) {
                std::cerr << "ERROR: Amount of priorities out of range\n";
                return false;
            }            
            if (*endptr != '\0') {
                std::cerr << "ERROR: Amount of priorities no numeric\n";
                return false;
            }
            options.rand_ps = ps;

            i++;
            if (i>=argc || argv[i][0] == '-') {
                std::cerr << "ERROR: Min amount of edges missing\n";
                return false;                    
            }
            int d1 = std::strtol(argv[i],&endptr,10);
            if (errno == ERANGE || d1 < 1 || d1 > 99) {
                std::cerr << "ERROR: Min amount of edges out of range\n";
                return false;
            }
            if (*endptr != '\0') {
                std::cerr << "ERROR: Min amount of edges  no numeric\n";
                return false;
            }
            options.rand_d1 = d1;

            i++;
            if (i>=argc || argv[i][0] == '-') {
                std::cerr << "ERROR: Max amount of edges missing\n";
                return false;                    
            }
            int d2 = std::strtol(argv[i],&endptr,10);
            if (errno == ERANGE || d2 < 2 || d2 > 100) {
                std::cerr << "ERROR: Max amount of edges out of range\n";
                return false;
            }
            if (*endptr != '\0') {
                std::cerr << "ERROR: Max amount of edges  no numeric\n";
                return false;
            }
            options.rand_d2 = d2;
        }
        else if (strcmp(argv[i],"--dzn")==0) {
            options.game_type = DZN;
            i++;
            if (i>=argc || argv[i][0] == '-') {
                std::cerr << "ERROR: DZN file name missing\n";
                return false;                    
            }
            options.game_filename = argv[i];                
        }
        else if (strcmp(argv[i],"--gm")==0) {
            options.game_type = GM;
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
        else if (strcmp(argv[i],"--export-dzn")==0) {
            i++;
            if (i>=argc || argv[i][0] == '-') {
                std::cerr << "ERROR: Target DZN file name missing\n";
                return false;                    
            }
            options.game_export_type = DZN;
            options.game_export_filename = argv[i];                
        }
        else if (strcmp(argv[i],"--export-gm")==0) {
            i++;
            if (i>=argc || argv[i][0] == '-') {
                std::cerr << "ERROR: Target GM file name missing\n";
                return false;                    
            }
            options.game_export_type = GM;
            options.game_export_filename = argv[i];                
        }
        else if (strcmp(argv[i],"--proof")==0) {
            options.game_proof = 1;
        }
        else if (strcmp(argv[i],"--matrix-based")==0) {
            options.game_proof = 2;
        }
        else if (strcmp(argv[i],"--help")==0) {
            std::cout << "Usage: " << argv[0] << " [options]\n";
            std::cout << "Options:\n";
            std::cout << "  --jurd <levels> <blocks>   : Jurdzinski game with <levels> levels and <blocks> blocks\n";
            std::cout << "  --dzn <filename>           : DZN file name\n";
            std::cout << "  --gm <filename>            : GM file name\n";
            std::cout << "  --rand <ns> <ps> <d1> <d2> : Random game\n";
            std::cout << "  --start <vertex>           : Starting vertex\n";
            std::cout << "  --print <type>             : Print game (0=Parity, 1=Parity+Time, 2=Verbose)\n";
            std::cout << "  --parity-even              : Search for play EVEN\n";
            std::cout << "  --parity-odd               : Search for play ODD\n";
            std::cout << "  --reward-max               : Seek to maximize the color\n";
            std::cout << "  --reward-min               : Seek to minimize the color\n";
            std::cout << "  --export-dzn <filename>    : Export game to DZN format (not solve)\n";
            std::cout << "  --export-gm <filename>     : Export game to GM format (not solve)\n";
            std::cout << "  --proof                    : Compare results of HRA using Matrix-based\n";
            return false;
        }
        else {
            std::cerr << "ERROR: Unknown option: " << argv[i] << std::endl;
            return false;
        }
    }
    return true;
}

//--------------------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    launchdebugstd();
    if (!parseOptions(argc, argv)) exit(1);
    Game* game = nullptr;

    //----------------------------------------------------------------------------------

    auto start = std::chrono::high_resolution_clock::now();

    switch (options.game_type) {
        case 1: // jurd
            game = new Game(options.jurd_levels, options.jurd_blocks, 
                            options.game_starts[0], options.game_reward);
            break;
        case 2: // dzn
            game = new Game(DZN, options.game_filename,
                            options.game_starts[0], options.game_reward);
            break;
        case 3: // gm
            game = new Game(GM, options.game_filename,
                            options.game_starts[0], options.game_reward);
            break;
        case 4: // random
            game = new Game(options.rand_ns, options.rand_ps,
                            options.rand_d1, options.rand_d2,
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

    if (options.game_export_type == DZN) {
        game->exportFile(DZN, options.game_export_filename);
    }
    else if (options.game_export_type == GM) {
        game->exportFile(GM, options.game_export_filename);
    }

    //----------------------------------------------------------------------------------

    while (!options.game_export_type && !options.game_proof) {
        start = std::chrono::high_resolution_clock::now();

        auto play = getPlay(*game, options.game_parity, options.game_starts[0]);

        end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> totaltime = end - start;

        if (options.game_print > 0) {
            std::cout << "Preparation time: " << preptime.count() << std::endl;
        }
        
        switch (options.game_print) {
            case 0:
                std::cout   << options.game_starts[0] << ": "
                            << (play==EVEN?"EVEN":"ODD") 
                            << std::endl;
                break;
            case 1: case 2:
                std::cout   << options.game_starts[0] << ": "
                            << (play==EVEN?"EVEN":"ODD") << " "
                            << totaltime.count() 
                            << std::endl; 
                break;
            default:
                break;
        }

        options.game_starts.erase(options.game_starts.begin());
        if (options.game_starts.size() == 0) break;
        game->setStart(options.game_starts[0]);
    }

    //----------------------------------------------------------------------------------

    if (options.game_proof==1) { //using matrix-based to proof HRA
        // game->reward = MAX;
        if (options.game_type != GM) {

        }
        Graph zlk(options.game_filename.c_str());
        auto res = zlk.Solve();
        std::cout << "Testing EVEN (" << res.first.size() <<  ") Faults: ";
        int counter=0;
        for (auto& r : res.first) {
            if (r>=game->nvertices) {
                std::cout << r << "*, ";
                counter++;
                continue;
            }
            game->start = r;
            auto play = getPlay(*game,EVEN,game->start);
            if (play != EVEN)   std::cout << r << ",";
            else                counter++;
        }
        std::cout << "  \n" << counter << "/" << res.first.size() << " ok"<< std::endl;

        std::cout << "Testing ODD (" << res.second.size() <<  ") Faults: ";
        counter=0;
        for (auto& r : res.second) {
            game->start = r;
            auto play = getPlay(*game,EVEN,game->start);
            if (play != ODD)   std::cout << r << ",";
            else                counter++;
        }
        std::cout << "  \n" << counter << "/" << res.second.size() << " ok" << std::endl;
    }

    //----------------------------------------------------------------------------------

    if (options.game_proof==2) { //matrix-based

        start = std::chrono::high_resolution_clock::now();

        Graph zlk(options.game_filename.c_str());

        end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> preptime = end - start;

        if (options.game_print > 0) {
            std::cout << "Preparation time: " << preptime.count() << std::endl;
        }

        start = std::chrono::high_resolution_clock::now();

        auto res = zlk.Solve();

        end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> totaltime = end - start;

        std::cout   << options.game_starts[0] << ": ";
        if (std::find(res.first.begin(), res.first.end(), game->start) != res.first.end()) {
            std::cout << "EVEN " ;
        } else {
            std::cout << "ODD " ; 
        }

        if (options.game_print >= 1) {
            std::cout << totaltime.count() << std::endl;
        }

        if (options.game_print >= 2) {
            std::cout << "EVENs=(";
            for (auto& r : res.first) { std::cout << r << " "; }
            std::cout << ")" << std::endl;

            std::cout << "ODDs=(";
            for (auto& r : res.second) {std::cout << r << " "; }
            std::cout << ")" << std::endl;
        }

        std::cout << std::endl;
    }

    //----------------------------------------------------------------------------------


    delete game;

    return 0;
}


