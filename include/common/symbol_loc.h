#pragma once

#include <cassert>
#include <memory>

namespace daisy {

struct InputFileInfo;
struct LocationContext;

struct TextPos {
    void nextLn() { ++ln, col = 1; }
    unsigned ln = 0, col = 0;
};

struct TextRange {
    const char* first = nullptr;
    const char* last = nullptr;
    TextPos pos;
};

struct SymbolLoc {
    SymbolLoc() = default;
    SymbolLoc(const LocationContext* ctx, TextPos p) : loc_ctx(ctx), first(p), last(p) {}
    SymbolLoc(const LocationContext* ctx, TextPos f, TextPos l) : loc_ctx(ctx), first(f), last(l) {}
    SymbolLoc& operator+=(const SymbolLoc& l);
    friend SymbolLoc operator+(const SymbolLoc& loc1, const SymbolLoc& loc2) {
        SymbolLoc loc = loc1;
        return loc += loc2;
    }
    const LocationContext* loc_ctx = nullptr;
    TextPos first, last;
};

struct TextExpansion {
    SymbolLoc loc;
};

struct LocationContext {
    LocationContext(const InputFileInfo* fl, const TextExpansion& exp) : file(fl), expansion(exp) {}
    const InputFileInfo* file;
    TextExpansion expansion;
};

}  // namespace daisy
