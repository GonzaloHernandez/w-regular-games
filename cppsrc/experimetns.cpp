
#include "cpsolver.cpp"
#include "satencoder.cpp"
#include <cstdlib>

#define time1 auto start = std::chrono::high_resolution_clock::now()  
#define time2 auto start = std::chrono::high_resolution_clock::now() 
#define duration std::chrono::duration<double, std::micro> duration = end - start; 

struct options {
    int print_game      = 0; 
    int game_type       = 0; // 0=default 1=jurd 2=dzn 3=gm
    int jurd_levels     = 2;
    int jurd_blocks     = 1;
    int game_start      = 0;
    int solving_type    = 1; // 1=cpsolver 2=satencoding
    int filter_type     = 1;
    std::string game_filename   = "";
    std::string dimacs_filename = "";
} ex;

bool parseExperimentOptions(int argc, char *argv[]) {
    for (int i=1; i<argc; i++) {
        if (argv[i][0] == '-') {
            if (strcmp(argv[i],"-jurd")==0) {
                ex.game_type = 1;
                i++;
                if (argv[i][0] == '-') continue;

                char* endptr;
                int levels = std::strtol(argv[i],&endptr,10);
                if (errno == ERANGE || levels < 2 || levels > INT_MAX) {
                    std::cerr << "ERROR: Jurdzinski level out of range";
                    return false;
                }
                if (*endptr != '\0') {
                    std::cerr << "ERROR: Jurdzinski level no numeric";
                    return false;
                }
                ex.jurd_levels = levels;

                i++;
                if (argv[i][0] == '-') continue;
                int blocks = std::strtol(argv[i],&endptr,10);
                if (errno == ERANGE || blocks < 1 || blocks > INT_MAX) {
                    std::cerr << "ERROR: Jurdzinski blocks out of range";
                    return false;
                }            
                if (*endptr != '\0') {
                    std::cerr << "ERROR: Jurdzinski blocks no numeric";
                    return false;
                }
                ex.jurd_blocks = blocks;
            }
            else if (strcmp(argv[i],"-dzn")==0) {
                ex.game_type = 2;
                i++;
                if (argv[i][0] == '-') {
                    std::cerr << "ERROR: DZN file name missing";
                    return false;                    
                }
                ex.game_filename = argv[i];                
            }
            else if (strcmp(argv[i],"-gm")==0) {
                ex.game_type = 3;
                i++;
                if (argv[i][0] == '-') {
                    std::cerr << "ERROR: GM file name missing";
                    return false;                    
                }
                ex.game_filename = argv[i];                
            }
            else if (strcmp(argv[i],"-start")==0) {
                i++;
                if (argv[i][0] == '-') {
                    std::cerr << "ERROR: Starting vertex missing";
                    return false;                    
                }
                char* endptr;
                int start = std::strtol(argv[i],&endptr,10);
                if (errno == ERANGE || start < 0 || start > INT_MAX) {
                    std::cerr << "ERROR: Starting vertex out of range";
                    return false;
                }
                if (*endptr != '\0') {
                    std::cerr << "ERROR: Starting vertex no numeric";
                    return false;
                }
                ex.game_start = start;
            }
            else if (strcmp(argv[i],"-solving")==0) {
                i++;
                if (argv[i][0] == '-') {
                    std::cerr << "ERROR: Solving purppose missing";
                    return false;                    
                }
                char* endptr;
                int solving = std::strtol(argv[i],&endptr,10);
                if (errno == ERANGE || solving < 1 || solving > 2) {
                    std::cerr << "ERROR: Solving purppose out of range";
                    return false;
                }
                if (*endptr != '\0') {
                    std::cerr << "ERROR: Solving purppose no numeric";
                    return false;
                }
                ex.solving_type = solving;
            }
            else if (strcmp(argv[i],"-filter")==0) {
                i++;
                if (argv[i][0] == '-') {
                    std::cerr << "ERROR: Filter number missing";
                    return false;
                }
                char* endptr;
                int filter = std::strtol(argv[i],&endptr,10);
                if (errno == ERANGE || filter < 1 || filter > 3) {
                    std::cerr << "ERROR: Odd Cycle Filter number out range (1,2,3)";
                    return false;
                }
                if (*endptr != '\0') {
                    std::cerr << "ERROR: Odd Cycle Filter no numeric";
                    return false;
                }
                ex.filter_type = filter;
            }
            else if (strcmp(argv[i],"-dimacs")==0) {
                i++;
                if (argv[i][0] == '-') {
                    std::cerr << "ERROR: DIMACS file name missing";
                    return false;                    
                }
                ex.dimacs_filename = argv[i];                
            }
        }
    }
    return true;
}

int main(int argc, char *argv[])
{
    parseOptions(argc, argv);
    Game* game = nullptr;

    // ------------------------------------------------------------
    // Creating a Jurdzinsky example dynamically
    // ------------------------------------------------------------
    // game = new Game(2,1,0);

    //------------------------------------------------------------
    // Loading a DZN file
    //------------------------------------------------------------
    // std::string filepath = "/home/chalo/Software/w-regular-games/data/";
    // std::string filename = filepath + "game-jurdzinski-3-2.dzn";
    // game = new Game(filename,0,Game::DZN);

    //------------------------------------------------------------
    // Loading a GM file
    //------------------------------------------------------------
    // std::string filepath = "/home/chalo/Deleteme/bes-benchmarks/gm/";
    // std::string filename = filepath + "jurdzinskigame_50_50.gm";
    // game = new Game(filename,7400,Game::GM);

    // ------------------------------------------------------------
    // Creating an example manually
    // ------------------------------------------------------------
    // game = new Game(
    //     {1,0,0,1,0,1,0},
    //     {2,1,2,2,4,3,4},
    //     {0,0,1,2,2,2,3,4,5,5,5,6},
    //     {1,2,2,0,3,5,2,5,2,4,6,5},
    //     0
    // );
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
            model = new CPModel(*game,ex.filter_type);
            so.nof_solutions = 1;
            engine.solve(model);
            delete model;
            break;
        }
        case 2: { // sat encoding 
            SATEncoder encoder(*game);
            auto cnf = encoder.getCNF();
            if (ex.dimacs_filename != "") {
                dimacs(cnf,ex.dimacs_filename);
            }
        }
        default:
            break;
    }

    game->printGame();

    delete game;

    return 0; 
}


// int main(int argc, char const *argv[])
// {
//     Game g("/home/chalo/Deleteme/bes-benchmarks/gm/jurdzinskigame_50_50.gm", 0, Game::GM);
//     // Game g(2,1, 0);
//     SATEncoder enc(g);
//     auto start = std::chrono::high_resolution_clock::now();
//     auto cnf = enc.getCNF();  // {{,},{,}}
//     auto end = std::chrono::high_resolution_clock::now();
//     std::chrono::duration<double, std::micro> duration = end - start; 
//     std::cout << "Encode time: " << duration.count() << std::endl;

//     start = std::chrono::high_resolution_clock::now();
//     dimacs(cnf, "/home/chalo/j_50_50.cnf");
//     end = std::chrono::high_resolution_clock::now();
//     duration = end - start; 
//     std::cout << "Saving time: " << duration.count() << std::endl;

//     return 0;
// }
