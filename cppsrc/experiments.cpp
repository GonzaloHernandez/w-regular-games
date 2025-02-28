
#include "cpsolver.cpp"
#include "satencoder.cpp"
#include <cstdlib>
#include <chrono>

struct options {
    int print_game      = 0; 
    int game_type       = 0; // 0=default 1=jurd 2=dzn 3=gm
    int jurd_levels     = 2;
    int jurd_blocks     = 1;
    int game_start      = 0;
    int solving_type    = 1; // 1=cpsolver 2=satencoding 3=zchaff 4=cadical
    int filter_type     = 1;
    std::string game_filename   = "";
    std::string dimacs_filename = "";
} ex;

//======================================================================================

bool parseExperimentOptions(int argc, char *argv[]) {
    for (int i=1; i<argc; i++) {
        if (argv[i][0] == '-') {
            if (strcmp(argv[i],"-jurd")==0) {
                ex.game_type = 1;
                if (i+1>=argc) return true;
                if (argv[i+1][0] == '-') continue;

                i++;
                char* endptr;
                int levels = std::strtol(argv[i],&endptr,10);
                if (errno == ERANGE || levels < 2 || levels > INT_MAX) {
                    std::cerr << "ERROR: Jurdzinski level out of range\n";
                    return false;
                }
                if (*endptr != '\0') {
                    std::cerr << "ERROR: Jurdzinski level no numeric\n";
                    return false;
                }
                ex.jurd_levels = levels;

                if (i+1>=argc) return true;
                if (argv[i+1][0] == '-') continue;
                i++;

                int blocks = std::strtol(argv[i],&endptr,10);
                if (errno == ERANGE || blocks < 1 || blocks > INT_MAX) {
                    std::cerr << "ERROR: Jurdzinski blocks out of range\n";
                    return false;
                }            
                if (*endptr != '\0') {
                    std::cerr << "ERROR: Jurdzinski blocks no numeric\n";
                    return false;
                }
                ex.jurd_blocks = blocks;
            }
            else if (strcmp(argv[i],"-dzn")==0) {
                ex.game_type = 2;
                i++;
                if (i>=argc || argv[i][0] == '-') {
                    std::cerr << "ERROR: DZN file name missing\n";
                    return false;                    
                }
                ex.game_filename = argv[i];                
            }
            else if (strcmp(argv[i],"-gm")==0) {
                ex.game_type = 3;
                i++;
                if (i>=argc || argv[i][0] == '-') {
                    std::cerr << "ERROR: GM file name missing\n";
                    return false;                    
                }
                ex.game_filename = argv[i];                
            }
            else if (strcmp(argv[i],"-start")==0) {
                i++;
                if (i>=argc || argv[i][0] == '-') {
                    std::cerr << "ERROR: Starting vertex missing\n";
                    return false;                    
                }
                char* endptr;
                int start = std::strtol(argv[i],&endptr,10);
                if (errno == ERANGE || start < 0 || start > INT_MAX) {
                    std::cerr << "ERROR: Starting vertex out of range\n";
                    return false;
                }
                if (*endptr != '\0') {
                    std::cerr << "ERROR: Starting vertex no numeric\n";
                    return false;
                }
                ex.game_start = start;
            }
            else if (strcmp(argv[i],"-solving")==0) {
                i++;
                if (i>=argc || argv[i][0] == '-') {
                    std::cerr << "ERROR: Solving purppose missing\n";
                    return false;                    
                }
                char* endptr;
                int solving = std::strtol(argv[i],&endptr,10);
                if (errno == ERANGE || solving < 1 || solving > 4) {
                    std::cerr << "ERROR: Solving purppose out of range\n";
                    return false;
                }
                if (*endptr != '\0') {
                    std::cerr << "ERROR: Solving purppose no numeric\n";
                    return false;
                }
                ex.solving_type = solving;
            }
            else if (strcmp(argv[i],"-filter")==0) {
                i++;
                if (i>=argc || argv[i][0] == '-') {
                    std::cerr << "ERROR: Filter number missing\n";
                    return false;
                }
                char* endptr;
                int filter = std::strtol(argv[i],&endptr,10);
                if (errno == ERANGE || filter < 1 || filter > 4) {
                    std::cerr << "ERROR: Odd Cycle Filter number out range (1,2,3,4)\n";
                    return false;
                }
                if (*endptr != '\0') {
                    std::cerr << "ERROR: Odd Cycle Filter no numeric\n";
                    return false;
                }
                ex.filter_type = filter;
            }
            else if (strcmp(argv[i],"-dimacs")==0) {
                i++;
                if (i>=argc || argv[i][0] == '-') {
                    std::cerr << "ERROR: DIMACS file name missing\n";
                    return false;                    
                }
                ex.dimacs_filename = argv[i];                
            }
        }
    }
    return true;
}

//======================================================================================

std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    using PcloseFunc = int (*)(FILE*);
    std::unique_ptr<FILE, PcloseFunc> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

//======================================================================================

int main(int argc, char *argv[])
{
    if (!parseExperimentOptions(argc, argv)) exit(1);
    Game* game = nullptr;

    //------------------------------------------------------------

    switch (ex.game_type) {
        case 1: // jurd
            game = new Game(ex.jurd_levels,ex.jurd_blocks,ex.game_start);
            break;
        case 2: // dzn
            game = new Game(ex.game_filename,ex.game_start,Game::DZN);
            break;
        case 3: // gm
            game = new Game(ex.game_filename,ex.game_start,Game::GM);
            break;
        default:
            game = new Game({0,1},{3,2},{0,1},{1,0},0);        
            break;
    }

    switch (ex.solving_type) {
        case 1: { // cp solver
            CPModel* model;
            model = new CPModel(*game, ex.filter_type);
            so.nof_solutions = 1;
            so.print_sol = false;
            engine.solve(model);
            auto total_time = std::chrono::duration_cast<std::chrono::milliseconds>(chuffed_clock::now() - engine.start_time);
            const std::chrono::milliseconds search_time = total_time - engine.init_time;
            
            std::cout << game->nvertices << " ";
            std::cout << game->nedges << " ";
            std::cout << engine.solutions << " ";
            std::cout << engine.init_time.count() << " ";
            std::cout << search_time.count() << std::endl;
            // std::cout << "[ nvertices nedges nsolutions milliseconds_init milliseconds_searching ]" << std::endl;
            
            delete model;
            break;
        }
        case 2: { // sat encoding 
            SATEncoder encoder(*game);
            auto start = std::chrono::high_resolution_clock::now();
            auto cnf = encoder.getCNF();
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> encodetime = end - start;

            start = std::chrono::high_resolution_clock::now();
            if (ex.dimacs_filename != "") {
                dimacs(cnf,ex.dimacs_filename);
            }
            end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> dimacstime = end - start;

            std::cout << game->nvertices << " ";
            std::cout << game->nedges << " ";
            std::cout << encodetime.count() << " ";
            std::cout << dimacstime.count() << std::endl;
            // std::cout << "[ nvertices nedges milliseconds_encode milliseconds_dimacs ]" << std::endl;   
            break;
        }
        case 3: { // sat zchaff
            SATEncoder encoder(*game);
            auto start = std::chrono::high_resolution_clock::now();
            auto cnf = encoder.getCNF();
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> encodetime = end - start;
            
            start = std::chrono::high_resolution_clock::now();
            dimacs(cnf,"temp.cnf");
            end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> dimacstime = end - start;
            
            auto output = exec("./zchaff temp.cnf | grep -E \"RESULT|Total Run Time\"");

            std::cout << game->nvertices << " ";
            std::cout << game->nedges << " ";
            std::cout << encodetime.count() << " ";
            std::cout << dimacstime.count() << " ";
            std::cout << output << std::endl;
            // std::cout << "[ nvertices nedges milliseconds_encode milliseconds_dimacs milliseconds_searching ]" << std::endl;   
            break;
        }
        case 4: { // sat cadical
            SATEncoder encoder(*game);
            auto start = std::chrono::high_resolution_clock::now();
            auto cnf = encoder.getCNF();
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> encodetime = end - start;
            
            start = std::chrono::high_resolution_clock::now();
            dimacs(cnf,"temp.cnf");
            end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> dimacstime = end - start;
            
            auto output = exec("./cadical temp.cnf | grep -E \"^s |total process time since initialization\"");

            std::cout << game->nvertices << " ";
            std::cout << game->nedges << " ";
            std::cout << encodetime.count() << " ";
            std::cout << dimacstime.count() << " ";
            std::cout << output << std::endl;
            // std::cout << "[ nvertices nedges milliseconds_encode milliseconds_dimacs output ]" << std::endl;   
            break;
        }
        default:
            break;
    }

    delete game;

    return 0; 
}