#include "common/symbol_loc.h"

using namespace daisy;

SymbolLoc& SymbolLoc::operator+=(const SymbolLoc& l) {
    if (l.loc_ctx == loc_ctx) {
        last = l.last;
        return *this;
    }

    // Find common location context

    auto get_expansion_level = [](const auto* loc) {
        unsigned level = 0;
        while (loc->loc_ctx) { loc = &loc->loc_ctx->expansion.loc, ++level; }
        return level;
    };

    const SymbolLoc* l1 = this;
    const SymbolLoc* l2 = &l;

    unsigned level1 = get_expansion_level(l1);
    unsigned level2 = get_expansion_level(l2);

    while (level1 < level2) { l2 = &l2->loc_ctx->expansion.loc, --level2; }
    while (level1 > level2) { l1 = &l1->loc_ctx->expansion.loc, --level1; }
    while (l1->loc_ctx != l2->loc_ctx) {
        assert(l1->loc_ctx && l2->loc_ctx);
        l1 = &l1->loc_ctx->expansion.loc;
        l2 = &l2->loc_ctx->expansion.loc;
    }

    // Symbol location in common context
    loc_ctx = l1->loc_ctx, first = l1->first, last = l2->last;
    return *this;
}
