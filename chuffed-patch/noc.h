enum parity_type    {EVEN,ODD};                             // 0,1
enum reward_type    {MIN,MAX};                              // 0,1
enum game_type      {DEF,JURD,RAND,MLADDER,DZN,GM,GMW,DIM}; // 0,1,2,3,4,5,6

//=============================================================================

parity_type opponent(parity_type PARITY) {
    if (PARITY==EVEN) return ODD; return EVEN;
}

//=============================================================================

class Game {
private:
    vec<int>        owners,priors,sources,targets;
    vec<vec<int>>   outs,ins;
    int             nvertices,nedges,init;
    reward_type     reward;

public:
    friend class NoOpponentCycle;
    //-------------------------------------------------------------------------

    Game(   vec<int>& _owners, vec<int>& _priors,
            vec<int>& _sources, vec<int>& _targets,
            int _init = 0, reward_type _reward = MAX)
    : init(_init), reward(_reward)
    {
        _owners .moveTo(owners);
        _priors .moveTo(priors);
        _sources.moveTo(sources);
        _targets.moveTo(targets);

        nvertices = owners.size();
        nedges    = sources.size();

        outs.growTo(nvertices);
        ins .growTo(nvertices);

        for(int i = 0; i < nedges; i++) {
            outs[sources[i]].push(i);
            ins [targets[i]].push(i);
        }
    }

    //-------------------------------------------------------------------------

    bool isBetterPriority(int p1,int p2) {
        if (reward==MIN && p1 < p2) return true; 
        if (reward==MAX && p1 > p2) return true;
        return false;
    }

};

//=============================================================================

class NoOpponentCycle : public Propagator {
private:
    Game& g;
    vec<BoolView> V;
    vec<BoolView> E;
    parity_type playerSAT;

    const int   CF_DONE     = 1;
    const int   CF_CONFLICT = 2;

public:
    //-------------------------------------------------------------------------
    NoOpponentCycle(Game& _g, vec<BoolView>& _V, vec<BoolView>& _E, 
        parity_type _playerSAT)
    : g(_g), V(_V), E(_E), playerSAT(_playerSAT)
    {
        for (int i=0; i<g.nvertices;i++) V[i].attach(this, 1 , EVENT_F );
        for (int i=0; i<g.nedges;   i++) E[i].attach(this, 1 , EVENT_F );
    }
    //-------------------------------------------------------------------------
    int findVertex(int vertex,vec<int>& path) {
        for (int i=0; i<path.size(); i++) {
            if (path[i] == vertex) return i;
        }
        return -1;
    }
    //-------------------------------------------------------------------------
    void clausify(vec<int>& path, vec<BoolView> &B, vec<Lit>& lits,int from) {
        for (int i=from; i<path.size()-1; i++) {
            lits.push(B[path[i]].getValLit());
        }
    }
    //-------------------------------------------------------------------------
    bool satisfy(vec<int>& pathV,vec<int>& pathE,int cycleIndex) {
        int m = g.priors[pathV[cycleIndex]];
        for (int i=cycleIndex+1; i<pathV.size(); i++) {
            if (g.isBetterPriority(g.priors[pathV[i]],m)) {
                m = g.priors[pathV[i]];
            }
        }
        return m%2==playerSAT;
    };
    //-------------------------------------------------------------------------
    int filterEager(vec<int>& pathV, vec<int>& pathE, int v, 
        int lastEdge, bool definedEdge) 
    {
        int index = findVertex(v,pathV);
        if (index >= 0) {
            if (!satisfy(pathV,pathE,index)) {
                vec<Lit> lits;
                lits.push();
                clausify(pathE,E,lits,0);
                Clause* reason = Reason_new(lits);
                if (! E[lastEdge].setVal(false,reason)) {
                    return CF_CONFLICT;
                }
            }
        }
        else if (definedEdge) {
            pathV.push(v);
            for (int i=0; i<g.outs[v].size(); i++) { int e = g.outs[v][i];
                if (E[e].isFalse()) continue;

                int w = g.targets[e];
                pathE.push(e);
                int status = filterEager(pathV, pathE, w, e, E[e].isTrue());
                pathE.pop();
                if (status == CF_CONFLICT) {
                    return status;
                }
            }
            pathV.pop();
        }
        return CF_DONE;
    }
    //-------------------------------------------------------------------------
    bool propagate() override {
        vec<int> pathV;
        vec<int> pathE;

        if (filterEager(pathV,pathE,g.init,-1,true) == CF_CONFLICT)
            return false;

        return true;
    }
    //-------------------------------------------------------------------------
    void wakeup(int i, int) override {
        pushInQueue();
    }
    //-------------------------------------------------------------------------
    void clearPropState() override {
        in_queue = false;
    }
};
