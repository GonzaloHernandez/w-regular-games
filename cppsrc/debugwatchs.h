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

inline std::string wVectorInt(std::vector<int>& data) {
    std::stringstream ss;
    ss << data;
    return ss.str();
}

// ================================================================================

inline std::ostream& operator<<(std::ostream& os, const vec<int>& obj) {
    os << "{";
    for (int i = 0; i < obj.size(); i++) {
        os << obj[i];
        if (i<obj.size()-1) os << ",";
    }
    os << "}";
    return os;
}

inline std::string wVecInt(vec<int>& data) {
    std::stringstream ss;
    ss << data;
    return ss.str();
}

// --------------------------------------------------------------------------------

inline std::ostream& operator<<(std::ostream& os, const vec<BoolView>& obj) {
    os << "{";
    for (int i = 0; i < obj.size(); i++) {
        if (obj[i].isFixed()) {
            os << (int)obj[i].isTrue();
        } else {
            os << " ";
        }
        if (i<obj.size()-1) os << ",";
    }
    os << "}";
    return os;
}

inline std::string wVecBoolView(vec<BoolView>& data) {
    std::stringstream ss;
    ss << data;
    return ss.str();
}

// --------------------------------------------------------------------------------

inline std::string wBoolView(BoolView& obj) {
    std::stringstream ss;
    ss << "B(";
    if (obj.isFixed()) {
        ss << (int)obj.isTrue();
    } else {
        ss << " ";
    }
    ss << ")";
    return ss.str();
}

// --------------------------------------------------------------------------------

inline void launchdebugwatchs() {
    std::vector<int> svi;
    wVectorInt(svi);

    vec<int> cvi(1);
    wVecInt(cvi);
    vec<BoolView> cvb(1);
    wVecBoolView(cvb);
    BoolView cb = newBoolVar();
    wBoolView(cb);
}

#endif