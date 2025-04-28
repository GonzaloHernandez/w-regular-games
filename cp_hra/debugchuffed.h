#ifndef debugchuffed_h
#define debugchuffed_h

#include <ostream>
#include <vector>
#include <sstream>

#ifndef modelling_h
#include "chuffed/vars/modelling.h"
#endif

// --------------------------------------------------------------------------------

inline std::ostream& operator<<(std::ostream& os, const vec<int>& obj) {
    os << "{";
    for (int i = 0; i < obj.size(); i++) {
        os << obj[i];
        if (i<obj.size()-1) os << ",";
    }
    os << "}";
    return os;
}

inline std::string wvi(vec<int>& data) {
    std::stringstream ss;
    ss << data;
    return ss.str();
}

// --------------------------------------------------------------------------------

inline std::ostream& operator<<(std::ostream& os, const vec<BoolView>& obj) {
    os << "{";
    for (int i = 0; i < obj.size(); i++) {
        if (i>0)                    os << ",";

        if (!obj[i].isFixed())      os << "_";
        else if (obj[i].isTrue())   os << "1";
        else                        os << "0";
    }
    os << "}";
    return os;
}

inline std::string wvb(vec<BoolView>& data) {
    std::stringstream ss;
    ss << data;
    return ss.str();
}

// --------------------------------------------------------------------------------

inline void launchdebugchuffed() {
    vec<int> vi;
    wvi(vi);

    vec<BoolView> vb;
    wvb(vb);
}

#endif