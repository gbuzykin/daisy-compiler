#pragma once

#include "common/symbol_loc.h"

#include <uxs/chars.h>

#include <algorithm>
#include <array>
#include <string_view>

namespace daisy {
namespace ir {

enum class IntType : unsigned { i8 = 0, u8, i16, u16, i32, u32, i64, u64 };
UXS_IMPLEMENT_BITWISE_OPS_FOR_ENUM(IntType);

constexpr std::array<std::string_view, 8> kIntTypeString = {"i8", "u8", "i16", "u16", "i32", "u32", "i64", "u64"};

class IntConst {
 public:
    IntConst() = default;
    static IntConst fromString(unsigned base, const SymbolLoc& loc, std::string_view s);
    static IntConst fromUInt64(IntType type, std::uint64_t v);
    void convert(IntType type) { *this = fromUInt64(type, v_); }

    IntType getType() const { return type_; }
    bool isSigned() const { return !(type_ & IntType::u8); }
    bool isZero() const { return v_ == 0; }
    template<typename Ty>
    Ty getValue() const {
        return static_cast<Ty>(v_);
    }

    friend IntConst operator-(IntConst c) { return fromUInt64(c.type_, ~c.v_ + 1); }
    friend IntConst operator+(IntConst c1, IntConst c2) {
        return fromUInt64(std::max(c1.type_, c2.type_), c1.v_ + c2.v_);
    }
    friend IntConst operator-(IntConst c1, IntConst c2) {
        return fromUInt64(std::max(c1.type_, c2.type_), c1.v_ - c2.v_);
    }
    friend IntConst operator*(IntConst c1, IntConst c2);
    friend IntConst operator/(IntConst c1, IntConst c2);
    friend IntConst operator%(IntConst c1, IntConst c2);

    friend IntConst operator~(IntConst c) { return fromUInt64(c.type_, ~c.v_); }
    friend IntConst operator&(IntConst c1, IntConst c2) {
        return fromUInt64(std::max(c1.type_, c2.type_), c1.v_ & c2.v_);
    }
    friend IntConst operator|(IntConst c1, IntConst c2) {
        return fromUInt64(std::max(c1.type_, c2.type_), c1.v_ | c2.v_);
    }
    friend IntConst operator^(IntConst c1, IntConst c2) {
        return fromUInt64(std::max(c1.type_, c2.type_), c1.v_ ^ c2.v_);
    }

    friend IntConst operator<<(IntConst c1, IntConst c2) {
        return IntConst::fromUInt64(c1.type_, c1.v_ << static_cast<unsigned>(c2.v_));
    }
    friend IntConst operator>>(IntConst c1, IntConst c2) {
        if (!c1.isSigned()) { return {c1.type_, c1.v_ >> static_cast<unsigned>(c2.v_)}; }
        return IntConst(c1.type_,
                        static_cast<std::uint64_t>(static_cast<std::int64_t>(c1.v_) >> static_cast<unsigned>(c2.v_)));
    }

    friend bool operator==(IntConst c1, IntConst c2);
    friend bool operator!=(IntConst c1, IntConst c2) { return !(c1 == c2); }

    friend bool operator<(IntConst c1, IntConst c2);
    friend bool operator<=(IntConst c1, IntConst c2) { return !(c2 < c1); }
    friend bool operator>=(IntConst c1, IntConst c2) { return !(c1 < c2); }
    friend bool operator>(IntConst c1, IntConst c2) { return c2 < c1; }

 private:
    IntConst(IntType type, std::uint64_t v) : type_(type), v_(v) {}

    template<unsigned base>
    static std::pair<const char*, bool> accumValue(const char* p, const char* p_end, std::uint64_t& v) {
        for (; p != p_end; ++p) {
            unsigned dig = uxs::dig_v(*p);
            if (dig >= base && *p != '_') { break; }
            std::uint64_t v_prev = v;
            v = base * v + dig;
            if (v < v_prev) { return {p, false}; }  // overflow
        }
        return {p, true};
    }

    IntType type_ = IntType::i32;
    std::uint64_t v_ = 0;
};

}  // namespace ir
}  // namespace daisy
