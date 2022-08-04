#pragma once

#include "common/symbol_loc.h"
#include "uxs/chars.h"

#include <string>

namespace daisy {

enum class IntegerType : unsigned { i8 = 0, i16 = 1, i32 = 3, i64 = 7, u8 = 8, u16 = 9, u32 = 11, u64 = 15 };
UXS_IMPLEMENT_BITWISE_OPS_FOR_ENUM(IntegerType, unsigned);

class IntegerConst {
 public:
    IntegerConst() = default;
    IntegerConst(IntegerType type, uint64_t v) : type_(type), v_(normalized(type, v)) {}
    static IntegerConst fromString(unsigned base, const SymbolLoc& loc, std::string_view s);
    void convert(IntegerType type) { type_ = type, v_ = normalized(type, v_); }

    IntegerType getType() const { return type_; }
    bool isSigned() const { return type_ < IntegerType::u8; }
    bool isZero() const { return v_ == 0; }
    template<typename Ty>
    Ty getValue() const {
        return static_cast<Ty>(v_);
    }

    friend IntegerConst operator-(IntegerConst c) { return {c.type_ & ~IntegerType::u8, ~c.v_ + 1}; }
    friend IntegerConst operator+(IntegerConst c1, IntegerConst c2) { return {c1.type_ | c2.type_, c1.v_ + c2.v_}; }
    friend IntegerConst operator-(IntegerConst c1, IntegerConst c2) { return {c1.type_ | c2.type_, c1.v_ - c2.v_}; }
    friend IntegerConst operator*(IntegerConst c1, IntegerConst c2);
    friend IntegerConst operator/(IntegerConst c1, IntegerConst c2);
    friend IntegerConst operator%(IntegerConst c1, IntegerConst c2);

    friend IntegerConst operator~(IntegerConst c) { return {c.type_, ~c.v_}; }
    friend IntegerConst operator&(IntegerConst c1, IntegerConst c2) { return {c1.type_ | c2.type_, c1.v_ & c2.v_}; }
    friend IntegerConst operator|(IntegerConst c1, IntegerConst c2) { return {c1.type_ | c2.type_, c1.v_ | c2.v_}; }
    friend IntegerConst operator^(IntegerConst c1, IntegerConst c2) { return {c1.type_ | c2.type_, c1.v_ ^ c2.v_}; }

    friend IntegerConst operator<<(IntegerConst c1, IntegerConst c2);
    friend IntegerConst operator>>(IntegerConst c1, IntegerConst c2);

    friend bool operator==(IntegerConst c1, IntegerConst c2) { return c1.v_ == c2.v_; }
    friend bool operator!=(IntegerConst c1, IntegerConst c2) { return !(c1 == c2); }

    friend bool operator<(IntegerConst c1, IntegerConst c2);
    friend bool operator<=(IntegerConst c1, IntegerConst c2) { return !(c2 < c1); }
    friend bool operator>=(IntegerConst c1, IntegerConst c2) { return !(c1 < c2); }
    friend bool operator>(IntegerConst c1, IntegerConst c2) { return c2 < c1; }

 private:
    static uint64_t normalized(IntegerType type, uint64_t v);

    template<unsigned base>
    static std::pair<const char*, bool> accumValue(const char* p, const char* p_end, uint64_t& v) {
        for (; p != p_end; ++p) {
            int dig = uxs::dig_v<base>(*p);
            if (dig < 0) { break; }
            uint64_t v_prev = v;
            v = base * v + dig;
            if (v < v_prev) { return {p, false}; }  // overflow
        }
        return {p, true};
    }

    IntegerType type_ = IntegerType::i32;
    uint64_t v_ = 0;
};

}  // namespace daisy
