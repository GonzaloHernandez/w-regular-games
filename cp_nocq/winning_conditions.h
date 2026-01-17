#ifndef WINNING_CONDITIONS_H
#define WINNING_CONDITIONS_H

#ifndef GAME_H
#include "game.h"
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
    virtual bool satisfy(int index,vec<int>& pathV,vec<int>& pathE) = 0;
};

//===========================================================================

class ParityCondition : public WinningCondition {
    using WinningCondition::WinningCondition;
public:

    bool satisfy(int index,vec<int>& pathV,vec<int>& pathE) override {
        int m = g.colors[pathV[index]];
        for (int i=index+1; i<pathV.size(); i++) {
            if (g.compareColors(g.colors[pathV[i]],m,BET)) {
                m = g.colors[pathV[i]];
            }
        }
        return m%2==playerSAT;
    };
};

//===========================================================================

class EnergyCondition : public WinningCondition {
    using WinningCondition::WinningCondition;
public:

    bool satisfy(int index,vec<int>& pathV,vec<int>& pathE) override {
        int sum = 0;
        for (int i=index; i<pathE.size(); i++) {
            sum += g.weights[pathE[i]];
        }
        
        if (playerSAT == EVEN) {
            return sum >= 0;
        }
        return sum < 0;
    };
};

//===========================================================================

class MeanPayoffCondition : public WinningCondition {
    using WinningCondition::WinningCondition;
private:
    int threshold;
public:

    void setThreshold(int t) { threshold = t; }

    bool satisfy(int index,vec<int>& pathV,vec<int>& pathE) override {
        int sum = 0;
        for (int i=index; i<pathE.size(); i++) {
            sum += g.weights[pathE[i]];
        }
        double avg = (double)sum / (double)(pathE.size() - index);

        if (playerSAT == EVEN) {
            return avg >= threshold;
        }
        return avg < threshold;
    };
};

#endif // WINNING_CONDITIONS_H