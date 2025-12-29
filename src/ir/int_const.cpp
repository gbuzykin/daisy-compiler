#include "ir/int_const.h"

#include "logger.h"

using namespace daisy;
using namespace ir;

/*static*/ IntConst IntConst::fromString(unsigned base, const SymbolLoc& loc, std::string_view s) {
    std::uint64_t v = 0;
    bool success = true;
    const char* p = s.data();
    switch (base) {
        case 2: std::tie(p, success) = accumValue<2>(p, s.data() + s.size(), v); break;
        case 8: std::tie(p, success) = accumValue<8>(p, s.data() + s.size(), v); break;
        case 10: std::tie(p, success) = accumValue<10>(p, s.data() + s.size(), v); break;
        case 16: std::tie(p, success) = accumValue<16>(p, s.data() + s.size(), v); break;
        default: assert(false);
    }
    if (!success) {
        logger::error(loc).println("integer literal is too large to be represented in any integer type");
        return {};
    }

    if (p == s.data() + s.size()) {  // no suffix
        if (base == 10) {
            if (v <= static_cast<std::uint64_t>(std::numeric_limits<std::int32_t>::max())) {
                return {IntType::i32, v};
            } else if (v <= static_cast<std::uint64_t>(std::numeric_limits<std::uint32_t>::max())) {
                return {IntType::u32, v};
            } else if (v <= static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())) {
                return {IntType::i64, v};
            }
            return {IntType::u64, v};
        } else if (v <= static_cast<std::uint64_t>(std::numeric_limits<std::uint32_t>::max())) {
            return {IntType::u32, v};
        }
        return {IntType::u64, v};
    }

    if (p + 1 == s.data() + s.size()) {  // 'i' (signed) or 'u' (unsigned) suffix
        if (*p == 'i') {
            if (v <= static_cast<std::uint64_t>(std::numeric_limits<std::int32_t>::max())) {
                return {IntType::i32, v};
            } else if (v <= static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())) {
                return {IntType::i64, v};
            }
            logger::error(loc).println("integer literal is too large to be represented in a signed integer type");
            return {};
        } else if (v <= static_cast<std::uint64_t>(std::numeric_limits<std::uint32_t>::max())) {
            return {IntType::u32, v};
        }
        return {IntType::u64, v};
    }

    // Type is explicitly specified
    IntType type = IntType::i32;
    switch (*(p + 1)) {
        case '8': {
            if (*p == 'i') {
                type = IntType::i8;
                if (v <= static_cast<std::uint64_t>(std::numeric_limits<std::int8_t>::max())) { return {type, v}; }
            } else {
                type = IntType::u8;
                if (v <= static_cast<std::uint64_t>(std::numeric_limits<std::uint8_t>::max())) { return {type, v}; }
            }
        } break;
        case '1': {
            if (*p == 'i') {
                type = IntType::i16;
                if (v <= static_cast<std::uint64_t>(std::numeric_limits<std::int16_t>::max())) { return {type, v}; }
            } else {
                type = IntType::u16;
                if (v <= static_cast<std::uint64_t>(std::numeric_limits<std::uint16_t>::max())) { return {type, v}; }
            }
        } break;
        case '6': {
            if (*p == 'i') {
                type = IntType::i64;
                if (v <= static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())) { return {type, v}; }
            }
            return {IntType::u64, v};
        } break;
        default: {
            if (*p == 'i') {
                type = IntType::i32;
                if (v <= static_cast<std::uint64_t>(std::numeric_limits<std::int32_t>::max())) { return {type, v}; }
            } else {
                type = IntType::u32;
                if (v <= static_cast<std::uint64_t>(std::numeric_limits<std::uint32_t>::max())) { return {type, v}; }
            }
        } break;
    }

    logger::error(loc).println("integer literal is too large to be represented in `{}` type",
                               kIntTypeString[static_cast<unsigned>(type)]);
    return {};
}

/*static*/ IntConst IntConst::fromUInt64(IntType type, std::uint64_t v) {
    switch (type) {
        case IntType::i8:
            return {type, static_cast<std::uint64_t>(static_cast<std::int64_t>(static_cast<std::int8_t>(v)))};
        case IntType::u8: return {type, v & 0xff};
        case IntType::i16:
            return {type, static_cast<std::uint64_t>(static_cast<std::int64_t>(static_cast<std::int16_t>(v)))};
        case IntType::u16: return {type, v & 0xffff};
        case IntType::i32:
            return {type, static_cast<std::uint64_t>(static_cast<std::int64_t>(static_cast<std::int32_t>(v)))};
        case IntType::u32: return {type, v & 0xffffffff};
        case IntType::i64:
        case IntType::u64: return {type, v};
    }
}

namespace daisy {
namespace ir {

IntConst operator*(IntConst c1, IntConst c2) {
    if (c1.type_ < c2.type_) {
        c1.convert(c2.type_);
    } else if (c1.type_ > c2.type_) {
        c2.convert(c1.type_);
    }
    if (!c1.isSigned()) { return IntConst::fromUInt64(c1.type_, c1.v_ * c2.v_); }
    return IntConst::fromUInt64(
        c1.type_, static_cast<std::uint64_t>(static_cast<std::int64_t>(c1.v_) * static_cast<std::int64_t>(c2.v_)));
}

IntConst operator/(IntConst c1, IntConst c2) {
    assert(c2.v_);
    if (c1.type_ < c2.type_) {
        c1.convert(c2.type_);
    } else if (c1.type_ > c2.type_) {
        c2.convert(c1.type_);
    }
    if (!c1.isSigned()) { return IntConst::fromUInt64(c1.type_, c1.v_ / c2.v_); }
    return IntConst::fromUInt64(
        c1.type_, static_cast<std::uint64_t>(static_cast<std::int64_t>(c1.v_) / static_cast<std::int64_t>(c2.v_)));
}

IntConst operator%(IntConst c1, IntConst c2) {
    assert(c2.v_);
    if (c1.type_ < c2.type_) {
        c1.convert(c2.type_);
    } else if (c1.type_ > c2.type_) {
        c2.convert(c1.type_);
    }
    if (!c1.isSigned()) { return IntConst::fromUInt64(c1.type_, c1.v_ % c2.v_); }
    return IntConst::fromUInt64(
        c1.type_, static_cast<std::uint64_t>(static_cast<std::int64_t>(c1.v_) % static_cast<std::int64_t>(c2.v_)));
}

bool operator==(IntConst c1, IntConst c2) {
    if (c1.type_ < c2.type_) {
        c1.convert(c2.type_);
    } else if (c1.type_ > c2.type_) {
        c2.convert(c1.type_);
    }
    return c1.v_ == c2.v_;
}

bool operator<(IntConst c1, IntConst c2) {
    if (c1.type_ < c2.type_) {
        c1.convert(c2.type_);
    } else if (c1.type_ > c2.type_) {
        c2.convert(c1.type_);
    }
    if (!c1.isSigned()) { return c1.v_ < c2.v_; }
    return static_cast<std::int64_t>(c1.v_) < static_cast<std::int64_t>(c2.v_);
}

}  // namespace ir
}  // namespace daisy
