#include "hrasolver.cpp"

struct op {
    int game_print_time = 0; 
    int game_print_game = 0; 
    int game_type       = 0; // 0=default 1=jurd 2=dzn 3=gm
    int jurd_levels     = 2;
    int jurd_blocks     = 1;
    int game_parity     = 0; // looking for 0=EVEN or 1=ODD
    reward_type game_reward         = MIN; 
    std::vector<int> game_starts    = {0};
    std::string game_filename       = "";
} op;

//======================================================================================

bool parseOps(int argc, char *argv[]) {
    for (int i=1; i<argc; i++) {
        if (strcmp(argv[i],"--jurd")==0) {
            op.game_type = 1;
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
            op.jurd_levels = levels;

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
            op.jurd_blocks = blocks;
        }
        else if (strcmp(argv[i],"--dzn")==0) {
            op.game_type = 2;
            i++;
            if (i>=argc || argv[i][0] == '-') {
                std::cerr << "ERROR: DZN file name missing\n";
                return false;                    
            }
            op.game_filename = argv[i];                
        }
        else if (strcmp(argv[i],"--gm")==0) {
            op.game_type = 3;
            i++;
            if (i>=argc || argv[i][0] == '-') {
                std::cerr << "ERROR: GM file name missing\n";
                return false;                    
            }
            op.game_filename = argv[i];                
        }
        else if (strcmp(argv[i],"--start")==0) {
            i++;
            if (i>=argc || argv[i][0] == '-') {
                std::cerr << "ERROR: Starting vertex missing\n";
                return false;                    
            }

            op.game_starts.clear();
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

                op.game_starts.push_back(std::stoi(item));
            }
        }
        else if (strcmp(argv[i],"--print-time")==0) {
            op.game_print_time = 1;
        }
        else if (strcmp(argv[i],"--print-game")==0) {
            op.game_print_game = 1;
        }
        else if (strcmp(argv[i],"--parity-even")==0) {
            op.game_parity = EVEN;
        }
        else if (strcmp(argv[i],"--parity-odd")==0) {
            op.game_parity = ODD;
        }
        else if (strcmp(argv[i],"--reward-max")==0) {
            op.game_reward = MAX;
        }
        else if (strcmp(argv[i],"--reward-min")==0) {
            op.game_reward = MIN;
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

//======================================================================================

int main(int argc, char *argv[])
{
    if (!parseOps(argc, argv)) exit(1);
    Game* game = nullptr;

    //------------------------------------------------------------

    auto start = std::chrono::high_resolution_clock::now();

    switch (op.game_type) {
        case 1: // jurd
            game = new Game(op.jurd_levels, op.jurd_blocks, 
                            op.game_starts[0], op.game_reward);
            break;
        case 2: // dzn
            game = new Game(Game::DZN, op.game_filename,
                            op.game_starts[0], op.game_reward);
            break;
        case 3: // gm
            game = new Game(Game::GM, op.game_filename,
                            op.game_starts[0], op.game_reward);
            break;
        default:
            game = new Game({0,1},{3,2},{0,1},{1,0},EVEN,MIN);
            break;
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> preptime = end - start;

    if (op.game_print_game) {
        game->printGame();
    }
    
    HRAModel* model;
    model = new HRAModel(*game);
    so.nof_solutions = 1;
    engine.solve(model);

    delete model;
    return 0;
}
