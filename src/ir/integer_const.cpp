#include "ir/integer_const.h"

#include "logger.h"

using namespace daisy;

/*static*/ IntegerConst IntegerConst::fromString(unsigned base, const SymbolLoc& loc, std::string_view s) {
    uint64_t v = 0;
    bool success = true;
    const char* p = s.data();
    switch (base) {
        case 2: std::tie(p, success) = accumValue<2>(p, s.data() + s.size(), v); break;
        case 8: std::tie(p, success) = accumValue<8>(p, s.data() + s.size(), v); break;
        case 16: std::tie(p, success) = accumValue<16>(p, s.data() + s.size(), v); break;
        case 10: std::tie(p, success) = accumValue<10>(p, s.data() + s.size(), v); break;
    }
    if (!success) {
        logger::error(loc).format("too large integer literal");
        return {};
    }
    IntegerType type = IntegerType::i32;
    if (p != s.data() + s.size() && (*p == 'i' || *p == 'u')) {
        if (p + 1 != s.data() + s.size()) {
            switch (*(p + 1)) {
                case '8': type = IntegerType::i8; break;
                case '1': type = IntegerType::i16; break;
                case '6': type = IntegerType::i64; break;
                default: break;
            }
        }
        if (*p == 'u') { type |= IntegerType::u8; }
    }
    IntegerConst c(type, v);
    if (c.isSigned()) {
        if (c.v_ != v || static_cast<int64_t>(c.v_) < 0) {
            logger::warning(loc).format("integer overflow, the value will be changed from {} to {}", v,
                                        static_cast<int64_t>(c.v_));
        }
    } else if (c.v_ != v) {
        logger::warning(loc).format("integer overflow, the value will be changed from {} to {}", v, c.v_);
    }
    return c;
}

/*static*/ uint64_t IntegerConst::normalized(IntegerType type, uint64_t v) {
    switch (type) {
        case IntegerType::i8: return static_cast<uint64_t>(static_cast<int64_t>(static_cast<int8_t>(v))); break;
        case IntegerType::i16: return static_cast<uint64_t>(static_cast<int64_t>(static_cast<int16_t>(v))); break;
        case IntegerType::i32: return static_cast<uint64_t>(static_cast<int64_t>(static_cast<int32_t>(v))); break;
        case IntegerType::u8: return v & 0xff; break;
        case IntegerType::u16: return v & 0xffff; break;
        case IntegerType::u32: return v & 0xffffffff; break;
        default: break;
    }
    return v;
}

namespace daisy {

IntegerConst operator*(IntegerConst c1, IntegerConst c2) {
    assert(c2.v_);
    c1.convert(c1.type_ | c2.type_);
    c2.convert(c1.type_);
    if (!c1.isSigned()) { return {c1.type_, c1.v_ * c2.v_}; }
    return IntegerConst(c1.type_, static_cast<uint64_t>(static_cast<int64_t>(c1.v_) * static_cast<int64_t>(c2.v_)));
}

IntegerConst operator/(IntegerConst c1, IntegerConst c2) {
    assert(c2.v_);
    c1.convert(c1.type_ | c2.type_);
    c2.convert(c1.type_);
    if (!c1.isSigned()) { return {c1.type_, c1.v_ / c2.v_}; }
    return IntegerConst(c1.type_, static_cast<uint64_t>(static_cast<int64_t>(c1.v_) / static_cast<int64_t>(c2.v_)));
}

IntegerConst operator%(IntegerConst c1, IntegerConst c2) {
    assert(c2.v_);
    c1.convert(c1.type_ | c2.type_);
    c2.convert(c1.type_);
    if (!c1.isSigned()) { return {c1.type_, c1.v_ % c2.v_}; }
    return IntegerConst(c1.type_, static_cast<uint64_t>(static_cast<int64_t>(c1.v_) % static_cast<int64_t>(c2.v_)));
}

IntegerConst operator<<(IntegerConst c1, IntegerConst c2) { return {c1.type_, c1.v_ << static_cast<unsigned>(c2.v_)}; }

IntegerConst operator>>(IntegerConst c1, IntegerConst c2) {
    if (!c1.isSigned()) { return {c1.type_, c1.v_ >> static_cast<unsigned>(c2.v_)}; }
    return IntegerConst(c1.type_, static_cast<uint64_t>(static_cast<int64_t>(c1.v_) >> static_cast<unsigned>(c2.v_)));
}

bool operator<(IntegerConst c1, IntegerConst c2) {
    if (!c1.isSigned() || c2.isSigned()) { return c1.v_ < c2.v_; }
    return static_cast<int64_t>(c1.v_) < static_cast<int64_t>(c2.v_);
}

}  // namespace daisy
