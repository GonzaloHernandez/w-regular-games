#include "iostream"
#include "chuffed/vars/modelling.h"
#include "chuffed/core/propagator.h"
#include "oddcyclefilter.cpp"
#include "initializer_list"
#include "chuffed/globals/dconnected.h"

#include "game.cpp"

#include <fstream>
#include <sstream>
#include <string>
#include <regex>

//----------------------------------------------------------------------

class CPSolver : public Problem {
public:
    static const int DZN    = 0;
    static const int GM     = 1;

private:
    Game& g;
    vec<BoolView> V;  
    vec<BoolView> E;

public:

    CPSolver(Game& g) : g(g) {
        V.growTo(g.nvertices);
        E.growTo(g.nedges);
        setupConstraints();
    }

    //----------------------------------------------------------------

    void setupConstraints() {

        for (int i=0; i<g.nvertices;  i++) V[i] = newBoolVar();
        for (int i=0; i<g.nedges;     i++) E[i] = newBoolVar();

        // Starting vertex
        fixVertices({g.start},{});

        // For every EVEN vertice, at least one outgoing edge must be activated
        for (int v=0; v<g.nvertices; v++) if (g.owners[v] == EVEN) {
            vec<Lit> clause;
            clause.push( V[v].getLit(false) );
            for (int e=0; e<g.nedges; e++) if (g.sources[e]==v) {
                clause.push( E[e].getLit(true) );
            }
            sat.addClause(clause);
        }

        // For every ODD vertice, each outgoing edge must be activated
        for (int v=0; v<g.nvertices; v++) if (g.owners[v] == ODD) {
            for (int e=0; e<g.nedges; e++) if (g.sources[e]==v) {
                vec<Lit> clause;
                clause.push( V[v].getLit(false) );        
                clause.push( E[e].getLit(true) );
                sat.addClause(clause);
            }
        }

        // For every activated edge, the source vertex must be activated
        for (int v=0; v<g.nvertices; v++) {
            for (int e=0; e<g.nedges; e++) if (g.sources[e]==v) {
                vec<Lit> clause;
                clause.push( E[e].getLit(false) );
                clause.push( V[v].getLit(true) );
                sat.addClause(clause);
            }
        }

        // For every activated edge, the target vertex must be activated
        for (int w=0; w<g.nvertices; w++) if (w != g.start) {
            for (int e=0; e<g.nedges; e++) if (g.targets[e]==w) {
                vec<Lit> clause;
                clause.push( E[e].getLit(false) );
                clause.push( V[w].getLit(true) );
                sat.addClause(clause);
            }
        }

        // Every infinite ODD play must be avoided.
        new CycleFilter(g.owners,g.colors,g.sources,g.targets,V,E,g.start);

        // Every unreachable vertex must be avoided.
        vec<vec<int>> _in, _out, _en;
        for (int v=0; v<g.owners.size(); v++) {
            _in.push();
            _out.push();
            for (int e=0; e<g.targets.size(); e++) {
                if (g.targets[e]==v) {
                    _in[v].push(e);
                }
                if (g.sources[e]==v) {
                    _out[v].push(e);
                }
            }
        }
        for (int e=0; e<g.targets.size(); e++) {
            _en.push();
            _en[e].push(g.sources[e]);
            _en[e].push(g.targets[e]);
        }
        new DReachabilityPropagator(g.start, V, E, _in, _out, _en);

        //------------------------------------------------------------

        vec<Branching*> bv(static_cast<unsigned int>(g.nvertices));
        vec<Branching*> be(static_cast<unsigned int>(g.nedges));
        for (int i = g.nvertices; (i--) != 0;) bv[i] = &V[i];
        for (int i = g.nedges;    (i--) != 0;) be[i] = &E[i];
        
        branch(bv, VAR_INORDER, VAL_MIN);
        branch(be, VAR_INORDER, VAL_MIN);
        output_vars(bv);
        output_vars(be);
    }

    //----------------------------------------------------------------

    void fixVertices(std::initializer_list<int> vs,std::initializer_list<int> nvs={}) {
        for (int v : vs) {
            vec<Lit> clause;
            clause.push(V[v].getLit(true));
            sat.addClause(clause);
        }
        for (int v : nvs) {
            vec<Lit> clause;
            clause.push(V[v].getLit(false));
            sat.addClause(clause);
        }
    }

    //----------------------------------------------------------------

    void fixEdges(std::initializer_list<int> es,std::initializer_list<int> nes={}) {
        for (int e : es) {
            vec<Lit> clause;
            clause.push(E[e].getLit(true));
            sat.addClause(clause);
        }
        for (int e : nes) {
            vec<Lit> clause;
            clause.push(E[e].getLit(false));
            sat.addClause(clause);
        }
    }

    //----------------------------------------------------------------

    void fixStartingZero() {
        for (int i=0; i<g.sources.size(); i++) {
            g.sources[i]--;
            g.targets[i]--;
        }
        // start--;
    }

    //----------------------------------------------------------------

    void print(std::ostream& out)   override {
        out << "V=[";
        bool first = true;
        for (int i=0; i<V.size(); i++) {
            if (V[i].isTrue()) {
                if (first) first=false; else out << ",";
                out << i;
            }
        }
        out << "]\nE=[";
        first = true;
        for (int i=0; i<E.size(); i++) {
            if (E[i].isTrue()) {
                if (first) first=false; else out << ",";
                out << i;
            }
        }
        out << "]";
    }
};

//----------------------------------------------------------------------

int main(int argc, char *argv[])
{
    parseOptions(argc, argv);
    Game* m = nullptr;

    goto gm;

    if (argc>1) {
        if (argc>1 && strcmp(argv[1],"jurd")==0) { jurd:
            // ------------------------------------------------------------
            // Creating a Jurdzinsky example dynamically
            // ------------------------------------------------------------

            int levels = 3;
            int blocks = 2;
            int start  = 0;

            if (argc==2) {
                std::cout << "Usage: ./wregulargames jurd [levels] [blocks] [start] [filtertype]" << std::endl;
                return 0;
            }
            if (argc>2) levels      = atoi(argv[2]);
            if (argc>3) blocks      = atoi(argv[3]);
            if (argc>4) start       = atoi(argv[4]);
            if (argc>5) filtertype  = atoi(argv[5]);

            m = new Game(levels,blocks,start);
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

            m = new Game(filename,start,Game::DZN);
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

            m = new Game(filename,start,Game::GM);
        }
    }
    else { none:
        // ------------------------------------------------------------
        // Creating an example manually
        // ------------------------------------------------------------
        int nvertices = 7;
        int owners[]  = {1,0,0,1,0,1,0};
        int colors[]  = {2,1,2,2,4,3,4};
        int nedges    = 12;
        int sources[] = {0,0,1,2,2,2,3,4,5,5,5,6};
        int targets[] = {1,2,2,0,3,5,2,5,2,4,6,5};

        int start     = 0;

        vec<int> _owners;
        vec<int> _colors;
        vec<int> _sources;
        vec<int> _targets;

        for (int i=0;i<g.nvertices; i++) {
            _owners.push(owners[i]);
            _colors.push(colors[i]);
        }

        for (int i=0;i<nedges; i++) {
            _sources.push(sources[i]);
            _targets.push(targets[i]);
        }
        
        m = new Game(_owners,_colors,_sources,_targets,start);
    }

    //------------------------------------------------------------

    so.nof_solutions = 1;

    engine.solve(m);
    delete m;
    return 0; 
}

//----------------------------------------------------------------------
