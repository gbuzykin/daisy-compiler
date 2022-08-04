#pragma once

#include "common/symbol_loc.h"

#include <forward_list>
#include <memory>
#include <string>
#include <vector>

namespace daisy {

struct CompilationContext;

struct InputFileInfo {
    InputFileInfo(CompilationContext* ctx, std::string name, size_t sz)
        : compilation_ctx(ctx), file_name(std::move(name)), text_size(sz) {}
    TextRange getText() const { return TextRange{text.get(), text.get() + text_size, TextPos{1, 1}}; }
    const CompilationContext* compilation_ctx;
    std::string file_name;
    size_t text_size;
    std::vector<std::string_view> text_lines;
    std::unique_ptr<char[]> text;
};

struct CompilationContext {
    explicit CompilationContext(std::string fname) : file_name(std::move(fname)) {}
    std::string file_name;
    std::forward_list<InputFileInfo> input_files;
    std::forward_list<LocationContext> loc_ctx_list;
    mutable unsigned warning_count = 0;
    mutable unsigned error_count = 0;
};

}  // namespace daisy
