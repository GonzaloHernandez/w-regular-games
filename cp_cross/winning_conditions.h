#ifndef WINNING_CONDITIONS_H
#define WINNING_CONDITIONS_H

#ifndef GAME_H
#include "../utils/game.h"
#endif

#include "chuffed/support/vec.h"

class WinningCondition {
protected:
    Game& g;
    parity_type playerSAT;

public:
    WinningCondition(Game& g, parity_type playerSAT=EVEN) 
    : g(g), playerSAT(playerSAT)
    {
    }
    virtual ~WinningCondition() = default;
    //-----------------------------------------------------------------------
    virtual bool satisfy(vec<int>& pathV,vec<int>& pathE,int cycleIndex) = 0;
};

//===========================================================================

class ParityCondition : public WinningCondition {
    using WinningCondition::WinningCondition;
public:

    bool satisfy(vec<int>& pathV,vec<int>& pathE,int cycleIndex) override {
        int m = g.priors[pathV[cycleIndex]];
        for (int i=cycleIndex+1; i<pathV.size(); i++) {
            if (g.comparePriorities(g.priors[pathV[i]],m)) {
                m = g.priors[pathV[i]];
            }
        }
        return m%2==playerSAT;
    };
};

//===========================================================================

class EnergyCondition : public WinningCondition {
    using WinningCondition::WinningCondition;
public:

    bool satisfy(vec<int>& pathV,vec<int>& pathE,int cycleIndex) override {
        float sum = 0.0f;
        for (int i=cycleIndex; i<pathE.size(); i++) {
            sum += g.weights[pathE[i]];
        }
        
        if (playerSAT == EVEN) {
            return sum >= 0.0f;
        }
        return sum < 0.0f;
    };
};

//===========================================================================

class MeanPayoffCondition : public WinningCondition {
    using WinningCondition::WinningCondition;
private:
    float threshold;
public:

    void setThreshold(float t) { threshold = t; }

    bool satisfy(vec<int>& pathV,vec<int>& pathE,int cycleIndex) override {
        float sum = 0.0f;
        for (int i=cycleIndex; i<pathE.size(); i++) {
            sum += g.weights[pathE[i]];
        }
        float avg = (float)sum / (float)(pathE.size() - cycleIndex);

        if (playerSAT == EVEN) {
            return avg >= threshold;
        }
        return avg < threshold;
    };
};

#endif // WINNING_CONDITIONS_H