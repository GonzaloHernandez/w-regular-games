#ifndef game_h
#include "game.h"
#endif

#include "chuffed/branching/branching.h"

class CoAttractorBranching : public Branching {
public:
    Game& g;
    vec<BoolView> Q;
public:
    //-----------------------------------------------------------------------
    CoAttractorBranching(Game& g, vec<BoolView>& Q)
    :   g(g), Q(Q)
    {  
    }

    bool finished() override {
    }

    DecInfo* branch() override {
    }
};
