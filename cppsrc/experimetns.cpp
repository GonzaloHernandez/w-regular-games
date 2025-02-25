
#include "cpsolver.cpp"
#include "satencoder.cpp"

struct options {
    int print_game      = 0; 
    int game_type       = 1; // 1=jurd 2=dzn 3=gm
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

    // goto manual;

    if (argc>1) {
        if (argc>1 && strcmp(argv[1],"jurd")==0) { jurd:
            // ------------------------------------------------------------
            // Creating a Jurdzinsky example dynamically
            // ------------------------------------------------------------

            int levels = 2;
            int blocks = 1;
            int start  = 0;

            if (argc==2) {
                std::cout << "Usage: ./wregulargames jurd [levels] [blocks] [start] [filtertype]" << std::endl;
                return 0;
            }
            if (argc>2) levels      = atoi(argv[2]);
            if (argc>3) blocks      = atoi(argv[3]);
            if (argc>4) start       = atoi(argv[4]);
            if (argc>5) filtertype  = atoi(argv[5]);

            game = new Game(levels,blocks,start);
        }
        else if (argc>1 &&strcmp(argv[1],"dzn")==0) { dzn:
            //------------------------------------------------------------
            // Loading a DZN file
            //------------------------------------------------------------

            std::string filepath = "/home/chalo/Software/w-regular-games/data/";
            std::string filename = filepath + "game-jurdzinski-3-2.dzn";

            int start   = 13;
            filtertype  = 2;

            if (argc==2) {
                std::cout << "Usage: ./wregulargames dzn [filename] [start] [filtertype]" << std::endl;
                return 0;
            }
            if (argc>2) filename    = argv[2];
            if (argc>3) start       = atoi(argv[3]);
            if (argc>4) filtertype  = atoi(argv[4]);

            game = new Game(filename,start,Game::DZN);
        }
        else if (argc>1 &&strcmp(argv[1],"gm")==0) { gm:
            //------------------------------------------------------------
            // Loading a GM file
            //------------------------------------------------------------

            std::string filepath = "/home/chalo/Deleteme/bes-benchmarks/gm/";
            std::string filename = filepath + "jurdzinskigame_50_50.gm";

            int start   = 7400;
            filtertype  = 1;

            if (argc==2) {
                std::cout << "Usage: ./wregulargames gm [filename] [start] [filtertype]" << std::endl;
                return 0;
            }
            if (argc>2) filename    = argv[2];
            if (argc>3) start       = atoi(argv[3]);
            if (argc>4) filtertype  = atoi(argv[4]);

            game = new Game(filename,start,Game::GM);
        }
        else {
            exit(0);
        }
    }
    else { manual:
        // ------------------------------------------------------------
        // Creating an example manually
        // ------------------------------------------------------------
        game = new Game(
            {1,0,0,1,0,1,0},
            {2,1,2,2,4,3,4},
            {0,0,1,2,2,2,3,4,5,5,5,6},
            {1,2,2,0,3,5,2,5,2,4,6,5},
            0
        );
    }

    //------------------------------------------------------------

    game->printGame();

    CPModel* model = new CPModel(*game);
    filtertype = 2;
    so.nof_solutions = 1;
    engine.solve(model);

    delete game;
    delete model;
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
