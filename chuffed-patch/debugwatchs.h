#ifndef debugwatchs_h
#define debugwatchs_h

#ifndef propagator_h
#include "chuffed/vars/modelling.h"
#endif

#ifndef vec_h
#include "chuffed/support/vec.h"
#endif

// --------------------------------------------------------------------------------

inline std::ostream& operator<<(std::ostream& os, const std::vector<int>& obj) {
    os << "{";
    for (int i = 0; i < obj.size(); i++) {
        os << obj[i];
        if (i<obj.size()-1) os << ",";
    }
    os << "}";
    return os;
}

inline std::string w(std::vector<int>& data) {
    std::stringstream ss;
    ss << data;
    return ss.str();
}

// --------------------------------------------------------------------------------


inline void launchdebugwatchs() {
    std::vector<int> vi;
    w(vi);
}

#endif