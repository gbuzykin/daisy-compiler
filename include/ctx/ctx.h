#pragma once

#include "common/symbol_loc.h"
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

struct CompilationContext {
    explicit CompilationContext(std::string fname) : file_name(std::move(fname)) {}
    std::string file_name;
    std::unordered_map<std::string, InputFileInfo> input_files;
    std::vector<std::string_view> include_paths;
    std::forward_list<LocationContext> loc_ctx_list;
    mutable unsigned warning_count = 0;
    mutable unsigned error_count = 0;
};

}  // namespace daisy
