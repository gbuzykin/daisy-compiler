#pragma once

#include "common/symbol_loc.h"
#include "ir/nodes/root_node.h"
#include "uxs/utility.h"

#include <forward_list>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace daisy {

struct CompilationContext;

struct InputFileInfo {
    enum class Flags : unsigned { kNone = 0, kOnce = 1 };
    InputFileInfo(CompilationContext* ctx, std::string fname) : compilation_ctx(ctx), file_name(std::move(fname)) {}
    TextRange getText() const { return TextRange{text.get(), text.get() + text_size, TextPos{1, 1}}; }
    const CompilationContext* compilation_ctx;
    std::string file_name;
    size_t text_size = 0;
    std::vector<std::string_view> text_lines;
    std::unique_ptr<char[]> text;
    mutable Flags flags = Flags::kNone;
};
UXS_IMPLEMENT_BITWISE_OPS_FOR_ENUM(InputFileInfo::Flags, unsigned);

struct MacroDefinition {
    enum class Type : unsigned { kUserDefined = 0, kBuiltIn };
    MacroDefinition(Type t, std::string_view i, bool v = false) : type(t), id(i), is_variadic(v) {}
    Type type;
    std::string_view id;
    bool is_variadic;
    SymbolLoc loc;
    TextRange text;
    std::unordered_map<std::string_view, std::pair<unsigned, SymbolLoc>> formal_args;
};
constexpr MacroDefinition::Type operator+(MacroDefinition::Type type, unsigned n) {
    return static_cast<MacroDefinition::Type>(static_cast<unsigned>(type) + n);
}
constexpr unsigned operator-(MacroDefinition::Type lhs, MacroDefinition::Type rhs) {
    return static_cast<unsigned>(lhs) - static_cast<unsigned>(rhs);
}

struct CompilationContext {
    explicit CompilationContext(std::string fname) : file_name(std::move(fname)) {}
    std::string file_name;
    std::unique_ptr<ir::RootNode> ir_root;
    std::unordered_map<std::string, std::unique_ptr<InputFileInfo>> input_files;
    std::vector<std::string_view> include_paths;
    std::unordered_map<std::string_view, std::unique_ptr<MacroDefinition>> macro_defs;
    std::forward_list<std::string> input_strings;
    std::forward_list<LocationContext> loc_ctx_list;
    mutable unsigned warning_count = 0;
    mutable unsigned error_count = 0;
};

}  // namespace daisy
