#pragma once

#include "common/symbol_loc.h"

#include <string_view>

namespace daisy {
namespace ir {

enum class FloatType : unsigned { f32 = 0, f64 = 1 };

class FloatConst {
 public:
    FloatConst() : type_(FloatType::f32) { v_.f32 = 0; }
    explicit FloatConst(float f32) : type_(FloatType::f32) { v_.f32 = f32; }
    explicit FloatConst(double f64) : type_(FloatType::f64) { v_.f64 = f64; }
    static FloatConst fromString(const SymbolLoc& loc, std::string_view s);

    FloatType getType() const { return type_; }
    bool isZero() const { return type_ == FloatType::f64 ? v_.f64 == 0 : v_.f32 == 0; }
    template<typename Ty>
    Ty getValue() const;

    friend FloatConst operator-(FloatConst c) {
        return c.type_ == FloatType::f64 ? FloatConst(-c.v_.f64) : FloatConst(-c.v_.f32);
    }
    friend FloatConst operator+(FloatConst c1, FloatConst c2);
    friend FloatConst operator-(FloatConst c1, FloatConst c2);
    friend FloatConst operator*(FloatConst c1, FloatConst c2);
    friend FloatConst operator/(FloatConst c1, FloatConst c2);

    friend bool operator==(FloatConst c1, FloatConst c2);
    friend bool operator!=(FloatConst c1, FloatConst c2) { return !(c1 == c2); }

    friend bool operator<(FloatConst c1, FloatConst c2);
    friend bool operator<=(FloatConst c1, FloatConst c2) { return !(c2 < c1); }
    friend bool operator>=(FloatConst c1, FloatConst c2) { return !(c1 < c2); }
    friend bool operator>(FloatConst c1, FloatConst c2) { return c2 < c1; }

 private:
    FloatType type_;
    union {
        float f32;
        double f64;
    } v_;
};

template<>
inline float FloatConst::getValue() const {
    return type_ == FloatType::f64 ? static_cast<float>(v_.f64) : v_.f32;
}

template<>
inline double FloatConst::getValue() const {
    return type_ == FloatType::f64 ? v_.f64 : static_cast<double>(v_.f32);
}

}  // namespace ir
}  // namespace daisy
