#include "cpsolver.cpp"

#include "../various/zielonka.cpp"
#include "../resources/debugchuffed.h"
#include "../resources/debugstd.h"

//--------------------------------------------------------------------------------------

struct options {
    bool print_time        = false; 
    bool print_game        = false; 
    bool print_solution    = false; 
    bool print_verbose     = false; 
    int game_type          = 0; // enum game_type{DEF,JURD,DZN,GM,RAND}
    std::vector<int>    vals            = {};
    reward_type         reward          = MAX; 
    std::vector<int>    starts          = {0};
    std::string         game_filename   = "";
    std::string         export_filename = "";
    int                 export_type     = 0; // 0=not export 2=DZN 3=GM
    int                 solver          = 5; // 1=CPModel 2=SAT-encoding 3=SAT-zchaff 4=SAT-cadical 5=ZRA
    int                 filter_type     = 3; // 1=Basic 2=Multi 3=Memo 4=Reload
    int                 proof           = 0; // 0=no 1=yes 2=eager
} options;

//--------------------------------------------------------------------------------------

bool parseMyOptions(int argc, char *argv[]) {
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
            options.vals.push_back(levels);

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
            options.vals.push_back(blocks);
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
            options.vals.push_back(ns);

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
            options.vals.push_back(ps);

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
            options.vals.push_back(d1);

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
            options.vals.push_back(d2);
        }
        else if (strcmp(argv[i],"--ladder")==0) {
            options.game_type = LADDER;
            i++;
            
            if (i>=argc || argv[i][0] == '-') {
                std::cerr << "ERROR: Number of blocks missing\n";
                return false;                    
            }
            char* endptr;
            int blocks = std::strtol(argv[i],&endptr,10);
            if (errno == ERANGE || blocks < 1 || blocks > 1000000) {
                std::cerr << "ERROR: Ladder blocks out of range\n";
                return false;
            }
            if (*endptr != '\0') {
                std::cerr << "ERROR: Ladder blocks no numeric\n";
                return false;
            }
            options.vals.push_back(blocks);
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

            options.starts.clear();
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

                options.starts.push_back(std::stoi(item));
            }
        }
        else if (strcmp(argv[i],"--export-dzn")==0) {
            i++;
            if (i>=argc || argv[i][0] == '-') {
                std::cerr << "ERROR: Target DZN file name missing\n";
                return false;                    
            }
            options.export_type = DZN;
            options.export_filename = argv[i];                
        }
        else if (strcmp(argv[i],"--export-gm")==0) {
            i++;
            if (i>=argc || argv[i][0] == '-') {
                std::cerr << "ERROR: Target GM file name missing\n";
                return false;                    
            }
            options.export_type = GM;
            options.export_filename = argv[i];                
        }
        else if (strcmp(argv[i],"--filter")==0) {
            i++;
            if (i>=argc || argv[i][0] == '-') {
                std::cerr << "ERROR: Number of vertices missing\n";
                return false;                    
            }
            char* endptr;
            int n = std::strtol(argv[i],&endptr,10);
            if (errno == ERANGE || n < 1 || n > 4) {
                std::cerr << "ERROR: Filter number out of range\n";
                return false;
            }
            if (*endptr != '\0') {
                std::cerr << "ERROR: Filter number is no numeric\n";
                return false;
            }
            options.filter_type = n;
        }
        else if (strcmp(argv[i],"--max")==0)            { options.reward = MAX; }
        else if (strcmp(argv[i],"--min")==0)            { options.reward = MIN; }

        else if (strcmp(argv[i],"--test")==0)           { options.solver = 0; }
        else if (strcmp(argv[i],"--cp")==0)             { options.solver = 1; }
        else if (strcmp(argv[i],"--sat-enc")==0)        { options.solver = 2; }
        else if (strcmp(argv[i],"--sat-zchaf")==0)      { options.solver = 3; }
        else if (strcmp(argv[i],"--sat_cadical")==0)    { options.solver = 4; }
        else if (strcmp(argv[i],"--zra")==0)            { options.solver = 5; }

        else if (strcmp(argv[i],"--proof")==0)          { options.proof = 1; }
        else if (strcmp(argv[i],"--proof-eager")==0)    { options.proof = 2; }
        else if (strcmp(argv[i],"--print-time")==0)     { options.print_time     = true; }
        else if (strcmp(argv[i],"--print-game")==0)     { options.print_game     = true; }
        else if (strcmp(argv[i],"--print-solution")==0) { options.print_solution = true; }
        else if (strcmp(argv[i],"--verbose")==0)        { options.print_verbose  = true; }

        else if (strcmp(argv[i],"--filter-basic")==0)   { options.filter_type = 1; }
        else if (strcmp(argv[i],"--filter-memo")==0)    { options.filter_type = 3; }
        else if (strcmp(argv[i],"--filter-reload")==0)  { options.filter_type = 4; }

        else if (strcmp(argv[i],"--help")==0) {
            std::cout << "Usage: " << argv[0] << " [options]\n";
            std::cout << "Options:\n";
            std::cout << "  --dzn <filename>           : DZN file name\n";
            std::cout << "  --gm <filename>            : GM file name\n";
            std::cout << "  --jurd <levels> <blocks>   : Jurdzinski game\n";
            std::cout << "  --rand <ns> <ps> <d1> <d2> : Random game\n";
            std::cout << "  --ladder <bl>              : Ladder game\n";
            std::cout << "  --start <vertex>           : Starting vertex\n";
            std::cout << "  --print-time               : Print time)\n";
            std::cout << "  --print-game               : Print game)\n";
            std::cout << "  --print-solution           : Print solution (All vertices)\n";
            std::cout << "  --verbose                  : Print everything\n";
            std::cout << "  --max                      : Seek to maximize the color\n";
            std::cout << "  --min                      : Seek to minimize the color\n";
            std::cout << "  --export-dzn <filename>    : Export game to DZN format (not solve)\n";
            std::cout << "  --export-gm <filename>     : Export game to GM format (not solve)\n"; 
            std::cout << "  --cp                       : Solve using CP (new method)\n";
            std::cout << "  --sat-encoding             : Encode on DIMACS\n";
            std::cout << "  --sat-zchaff               : Solve using zChaff\n";
            std::cout << "  --sat-cadical              : Solve using Cadical\n";
            std::cout << "  --proof                    : Compare results using Zielonka\n";
            std::cout << "  --proof-eager              : Looking for counterexample\n";
            std::cout << "  --filter-basic             : Filter of NOC\n";
            std::cout << "  --filter-memo              : Filter of NOC\n";
            std::cout << "  --filter-reload            : Filter of NOC\n";
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
    // launchdebugstd();
    // launchdebugchuffed();
    if (!parseMyOptions(argc, argv)) exit(1);
    Game* game = nullptr;

    //----------------------------------------------------------------------------------

    auto start = std::chrono::high_resolution_clock::now();

    switch (options.game_type) {
        case 1: // jurd
            game = new Game(JURD, options.vals, options.starts[0], options.reward);
            break;
        case 2: // dzn
            game = new Game(DZN, options.game_filename, options.starts[0], options.reward);
            break;
        case 3: // gm
            game = new Game(GM, options.game_filename, options.starts[0], options.reward);
            break;
        case 4: // random
            game = new Game(RAND, options.vals, options.starts[0], options.reward);
            break;
        case 5: // ladder
            game = new Game(LADDER, options.vals, options.starts[0], options.reward);
            break;
        default:
            game = new Game({0,1},{3,2},{0,1},{1,0},EVEN,MIN);
            break;
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> launchinggame = end - start;

    if ((options.print_game || options.print_verbose) && options.proof<2) {
        game->printGame();
    }

    if ((options.print_time || options.print_verbose) && options.proof<2) {
        std::cout << "Game creation time : " << launchinggame.count() << std::endl;
    }

    if (options.export_type == DZN && options.game_type != DZN && options.proof<2) {
        game->exportFile(DZN, options.export_filename);
    }
    else if (options.export_type == GM && options.game_type != GM && options.proof<2) {
        game->exportFile(GM, options.export_filename);
    }

    //----------------------------------------------------------------------------------

    if (options.solver==0) {
        // For testing purpposes
    }

    //----------------------------------------------------------------------------------

    else if (options.proof==1 && options.solver==1) { //using Zielonka for proof cp
        start = std::chrono::high_resolution_clock::now();
        Zielonka zlk(*game);

        CPModel* model;
        model = new CPModel(*game, options.filter_type, false, options.print_solution);
        so.print_sol = true;
        so.nof_solutions = 0;

        end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> preptime = end - start;

        if (options.print_time || options.print_verbose) {
            std::cout << "Preparation time   : " << preptime.count() << std::endl;
        }

        start = std::chrono::high_resolution_clock::now();
        auto win = zlk.solve();
        end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> totaltime1 = end - start;
        
        if (options.print_time || options.print_verbose) {
            std::cout << "Solving time (ZRA) : " << totaltime1.count() << std::endl;
        }

        start = std::chrono::high_resolution_clock::now();

        std::streambuf* old_buf = std::cout.rdbuf();
        std::ofstream null_stream("/dev/null");  // or "NUL" on Windows
        std::cout.rdbuf(null_stream.rdbuf());

        engine.solve(model);

        std::cout.rdbuf(old_buf);

        end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> totaltime2 = end - start;

        if (options.print_time || options.print_verbose) {
            std::cout << "Solving time (CP)  : " << totaltime2.count() << std::endl;
        }

        if (engine.solutions>0 && 
            std::find(win[0].begin(), win[0].end(), game->start) != win[0].end()) 
        {
                std::cout << ".";
        }
        else if (engine.solutions==0 && 
            std::find(win[1].begin(), win[1].end(), game->start) != win[1].end()) 
        {
            std::cout << ".";
        }
        else {
            std::cout << "Counter example found (Starting at " << game->start << "):" << std::endl;
            if (options.export_type == DZN) {
                game->exportFile(DZN, options.export_filename);
            }
            else if (options.export_type == GM) {
                game->exportFile(GM, options.export_filename);
            }

            if (options.print_game || options.print_verbose) {
                game->printGame();
            }            
        }

        delete model;
    }
    
    //----------------------------------------------------------------------------------

    //Looking for counterexample using Zielonka
    else if (options.proof==2 && options.solver==5) { 

        // if (options.game_type == RAND) {
        //     Zielonka zlk(*game);

        //     HRAModel* model = new HRAModel(*game,true);
        //     so.print_sol = true;
        //     so.nof_solutions = 0;

        //     auto win = zlk.solve();

        //     std::streambuf* old_buf = std::cout.rdbuf();
        //     std::ofstream null_stream("/dev/null");  // or "NUL" on Windows
        //     std::cout.rdbuf(null_stream.rdbuf());

        //     engine.solve(model);

        //     std::cout.rdbuf(old_buf);

        //     int counter1=0;
        //     for (auto& r : win[0]) {
        //         if (model->getVal(r)!=EVEN) counter1++;
        //     }

        //     int counter2=0;
        //     for (auto& r : win[1]) {
        //         if (model->getVal(r)!=ODD) counter2++;
        //     }

        //     delete model;

        //     if (counter1>0 || counter2>0) {
        //         std::cout << "Counter example found:" << std::endl;
        //         if (options.export_type == DZN) {
        //             game->exportFile(DZN, options.export_filename);
        //         }
        //         else if (options.export_type == GM) {
        //             game->exportFile(GM, options.export_filename);
        //         }

        //         if (options.print_game || options.print_verbose) {
        //             game->printGame();
        //         }
        //     }
        //     else {
        //         std::cout << ".";
        //     }
        // }
    }

    //----------------------------------------------------------------------------------

    else if (options.solver==1) { //CP-NOC
            start = std::chrono::high_resolution_clock::now();
            CPModel* model;
            model = new CPModel(*game, options.filter_type, false, options.print_solution);
            so.nof_solutions = 1;
            so.sym_static = true;
            so.print_sol = options.print_solution==0?false:true;
            end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> preptime = end - start;            

            start = std::chrono::high_resolution_clock::now();

            if (!options.print_solution && !options.print_verbose) {
                std::streambuf* old_buf = std::cout.rdbuf();
                std::ofstream null_stream("/dev/null");
                std::cout.rdbuf(null_stream.rdbuf());
                engine.solve(model);
                std::cout.rdbuf(old_buf);
            }
            else {
                engine.solve(model);
            }

            end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> totaltime = end - start;
            
            if (options.print_time || options.print_verbose) {
                std::cout << "Solving time       : " << totaltime.count() << std::endl;
            }   

            if (!options.print_solution && !options.print_verbose) {
                std::cout << (engine.solutions>0?"EVEN":"ODD") << " ";
                std::cout << engine.init_time.count()/1000.0 << " ";
                std::cout << totaltime.count()/1000.0 << " ";
                std::cout << memUsed() << std::endl;
            }
            else {
                std::cout << totaltime.count()/1000.0 << std::endl;
            }
            
            delete model;
    }

    //----------------------------------------------------------------------------------

    else if (options.solver==5) { //ZRA
        start = std::chrono::high_resolution_clock::now();

        Zielonka zlk(*game);
        
        end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> preptime = end - start;

        if (options.print_time || options.print_verbose) {
            std::cout << "Preparation time   : " << preptime.count() << std::endl;
        }

        start = std::chrono::high_resolution_clock::now();
        auto win = zlk.solve();
        end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> totaltime = end - start;

        if (options.print_time || options.print_verbose) {
            std::cout << "Solving time       : " << totaltime.count() << std::endl;
        }

        if (options.print_solution || options.print_verbose) {
            std::cout << "EVEN " << win[0] << std::endl;
            std::cout << "ODD  " << win[1] << std::endl;
        }

        for(auto& v0 : options.starts) {
            auto it = std::find(win[0].begin(), win[0].end(), v0);

            std::cout   << v0 << ": ";
            if (it != win[0].end()) {
                std::cout << "EVEN ";
            } else {
                std::cout << "ODD ";
            }

            std::cout << std::endl;            
        }
    }

    //----------------------------------------------------------------------------------

    delete game;

    return 0;
}


