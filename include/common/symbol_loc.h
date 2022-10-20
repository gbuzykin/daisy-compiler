#pragma once

#include <utility>

namespace daisy {

struct InputFileInfo;
struct LocationContext;
struct MacroDefinition;

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
    friend SymbolLoc operator+(const SymbolLoc& l1, const SymbolLoc& l2) {
        SymbolLoc l = l1;
        return l += l2;
    }
    const LocationContext* loc_ctx = nullptr;
    TextPos first, last;
};

struct TextExpansion {
    SymbolLoc loc;
    const MacroDefinition* macro_def = nullptr;
};

struct LocationContext {
    template<typename... Args>
    LocationContext(const InputFileInfo* f, Args&&... expansion_args)
        : file(f), expansion{std::forward<Args>(expansion_args)...} {}
    const InputFileInfo* file;
    TextExpansion expansion;
};

}  // namespace daisy
