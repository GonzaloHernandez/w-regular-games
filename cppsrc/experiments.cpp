#include "cpsolver.cpp"
#include "satencoder.cpp"
#include <cstdlib>
#include <chrono>
#include "Graph.h"

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

//----------------------------------------------------------------------
std::ostream& operator<<(std::ostream& os, const std::vector<int>& vec) {
    os << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        os << vec[i];
        if (i != vec.size() - 1) os << ", ";
    }
    os << "]";
    return os;
}

void debug() {   
    std::vector<int> viii;
    std::cout << viii << std::endl;
    vec<bool> vb;
    vb.dbg();
    vec<int> vi;
    vi.dbg();
    vec<vec<int>> vvi;
    vvi.dbg();
    vec<Lit> vl;
    vl.dbg();
    vec<vec<Lit>> vvl;
    vvl.dbg();
    vec<vec<WatchElem>> vvw;
    vvw.dbg();
    vec<Reason> vr;
    vr.dbg();
    vec<ChannelInfo> vci;
    vci.dbg();
    vec<int8_t> vi8;
    vi8.dbg();
    vec<Clause*> vc;
    vc.dbg();
    vec<vec<Clause*>> vvc;
    vvc.dbg();
    vec<IntVar*> viv;
    viv.dbg();
    vec<TrailElem> vt;
    vt.dbg();
    vec<Propagator*> vpr;
    vpr.dbg();
    vec<vec<Propagator*>> vvpr;
    vvpr.dbg();
    vec<DecInfo> vdi;
    vdi.dbg();
    vec<BoolView> vbv;
    vbv.dbg();
    vec<Lit> lss;
    Clause cla(lss,false);
    cla.dbg();
    Lit l;
    l.dbg();
    BoolView b;
    b.dbg();
    sat.seen.dbg();
    sat.assignsdbg();
}

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
                if (errno == ERANGE || solving < 1 || solving > 6) {
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
            model = new CPModel(*game, ex.filter_type,2);
            so.nof_solutions = 1;
            so.print_sol = true;
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
                encoder.dimacs(cnf,ex.dimacs_filename);
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
            encoder.dimacs(cnf,"temp.cnf");
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
            encoder.dimacs(cnf,"temp.cnf");
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
        case 5: { // zielonka
            Graph zlk(ex.game_filename.c_str());
            auto start = std::chrono::high_resolution_clock::now();
            auto res = zlk.Solve();
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> zlktime = end - start;

            if (std::find(res.first.begin(), res.first.end(), ex.game_start) != res.first.end()) {
                std::cout << "Even wins" << " "; 
            } else {
                std::cout << "Odd wins" << " "; 
            }
            std::cout << zlktime.count() << std::endl;

            std::cout << "evens=(";
            for (auto& r : res.first) {
                std::cout << r << " ";
            }
            std::cout << ")" << std::endl;
            std::cout << "odds=(";
            for (auto& r : res.second) {
                std::cout << r << " ";
            }
            std::cout << ")" << std::endl;

            break;
        }
        case 6: { // testing cp using zielonka
            Graph zlk(ex.game_filename.c_str());
            auto res = zlk.Solve();

            std::cout << "Total Vertices for Even player: " << res.first.size() << std::endl;
            std::cout << "Total Vertices for Odd player:  " << res.second.size() << std::endl;

            std::cout << "---- Testing Event plays -----" << std::endl;
            for (auto& r : res.first) {
                CPModel* model;
                game->start = r;
                model = new CPModel(*game);
                so.nof_solutions = 1;
                so.print_sol = false;
                engine.solve(model);
                delete model;
                std::cout << "Staring at " << r << " winner: " << (engine.solutions>0?"Even":"Odd") << std::endl;
            }

            std::cout << "---- Testing Odd plays -----" << std::endl;
            for (auto& r : res.second) {
                CPModel* model;
                game->start = r;
                model = new CPModel(*game, ex.filter_type);
                so.nof_solutions = 1;
                so.print_sol = false;
                engine.solve(model);
                delete model;
                std::cout << "Staring at " << r << " winner: " << (engine.solutions>0?"Even":"Odd") << std::endl;
            }
            break;
        }
        default:
            break;
    }

    delete game;

    return 0; 
}
