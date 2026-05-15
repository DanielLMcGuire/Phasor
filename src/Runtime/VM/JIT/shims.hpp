#pragma once

#include "Value.hpp"

extern "C"
{
    inline bool  value_is_int  (Phasor::Value* v) { return v->isInt();   }
    inline bool  value_is_float(Phasor::Value* v) { return v->isFloat(); }
    inline bool  value_is_bool (Phasor::Value* v) { return v->isBool();  }

    inline int64_t value_get_int  (Phasor::Value* v) { return v->asInt();   }
    inline double  value_get_float(Phasor::Value* v) { return v->asFloat(); }
    inline bool    value_get_bool (Phasor::Value* v) { return v->asBool();  }

    inline void value_set_int  (Phasor::Value* v, int64_t i) { *v = Phasor::Value(i);    }
    inline void value_set_float(Phasor::Value* v, double  d) { *v = Phasor::Value(d);    }
    inline void value_set_bool (Phasor::Value* v, bool    b) { *v = Phasor::Value(b);    }

    inline void value_copy(Phasor::Value* dst, Phasor::Value* src) { *dst = *src; }
}