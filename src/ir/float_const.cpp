#include "ir/float_const.h"

#include "logger.h"

using namespace daisy;
using namespace ir;

/*static*/ FloatConst FloatConst::fromString(const SymbolLoc& loc, std::string_view s) {
    if (s.back() == 'f' || (s.size() > 3 && s[s.size() - 3] == 'f' && s[s.size() - 2] == '3')) {
        return FloatConst(uxs::from_string<float>(s));
    }
    return FloatConst(uxs::from_string<double>(s));
}

namespace daisy {
namespace ir {

FloatConst operator+(FloatConst c1, FloatConst c2) {
    if (c1.type_ == FloatType::f32 && c2.type_ == FloatType::f32) {
        return FloatConst(c1.v_.f32 + c2.v_.f32);
    } else if (c1.type_ < c2.type_) {
        c1.v_.f64 = static_cast<double>(c1.v_.f32);
    } else if (c1.type_ > c2.type_) {
        c2.v_.f64 = static_cast<double>(c2.v_.f32);
    }
    return FloatConst(c1.v_.f64 + c2.v_.f64);
}

FloatConst operator-(FloatConst c1, FloatConst c2) {
    if (c1.type_ == FloatType::f32 && c2.type_ == FloatType::f32) {
        return FloatConst(c1.v_.f32 - c2.v_.f32);
    } else if (c1.type_ < c2.type_) {
        c1.v_.f64 = static_cast<double>(c1.v_.f32);
    } else if (c1.type_ > c2.type_) {
        c2.v_.f64 = static_cast<double>(c2.v_.f32);
    }
    return FloatConst(c1.v_.f64 - c2.v_.f64);
}

FloatConst operator*(FloatConst c1, FloatConst c2) {
    if (c1.type_ == FloatType::f32 && c2.type_ == FloatType::f32) {
        return FloatConst(c1.v_.f32 * c2.v_.f32);
    } else if (c1.type_ < c2.type_) {
        c1.v_.f64 = static_cast<double>(c1.v_.f32);
    } else if (c1.type_ > c2.type_) {
        c2.v_.f64 = static_cast<double>(c2.v_.f32);
    }
    return FloatConst(c1.v_.f64 * c2.v_.f64);
}

FloatConst operator/(FloatConst c1, FloatConst c2) {
    if (c1.type_ == FloatType::f32 && c2.type_ == FloatType::f32) {
        assert(c2.v_.f32 != 0);
        return FloatConst(c1.v_.f32 / c2.v_.f32);
    } else if (c1.type_ < c2.type_) {
        c1.v_.f64 = static_cast<double>(c1.v_.f32);
    } else if (c1.type_ > c2.type_) {
        c2.v_.f64 = static_cast<double>(c2.v_.f32);
    }
    assert(c2.v_.f64 != 0);
    return FloatConst(c1.v_.f64 / c2.v_.f64);
}

bool operator==(FloatConst c1, FloatConst c2) {
    if (c1.type_ == FloatType::f32 && c2.type_ == FloatType::f32) {
        return c1.v_.f32 == c2.v_.f32;
    } else if (c1.type_ < c2.type_) {
        c1.v_.f64 = static_cast<double>(c1.v_.f32);
    } else if (c1.type_ > c2.type_) {
        c2.v_.f64 = static_cast<double>(c2.v_.f32);
    }
    return c1.v_.f64 == c2.v_.f64;
}

bool operator<(FloatConst c1, FloatConst c2) {
    if (c1.type_ == FloatType::f32 && c2.type_ == FloatType::f32) {
        return c1.v_.f32 < c2.v_.f32;
    } else if (c1.type_ < c2.type_) {
        c1.v_.f64 = static_cast<double>(c1.v_.f32);
    } else if (c1.type_ > c2.type_) {
        c2.v_.f64 = static_cast<double>(c2.v_.f32);
    }
    return c1.v_.f64 < c2.v_.f64;
}

}  // namespace ir
}  // namespace daisy
