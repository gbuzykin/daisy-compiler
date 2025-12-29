#include "../daisy_parser_pass.h"
#include "logger.h"

#include <filesystem>

using namespace daisy;

namespace {

void parseIncludeDirective(DaisyParserPass* pass, SymbolInfo& tkn) {
    int tt = pass->lex(tkn);  // Parse reference file path
    if (tt != parser_detail::tt_string_literal) {
        logger::error(tkn.loc).println("expected file name");
        return;
    }

    auto file_name = std::move(std::get<std::string>(tkn.val));
    const auto& include_paths = pass->getCompilationContext().include_paths;

    std::filesystem::path path(tkn.loc.loc_ctx->file->file_name);
    auto include_path_it = include_paths.begin();
    SymbolLoc expansion_loc = tkn.loc;

    pass->ensureEndOfInput(tkn);
    path = path.has_parent_path() ? path.parent_path() : "./";

    while (true) {
        path /= file_name;
        if (pass->pushInputFile(path.generic_string(), expansion_loc)) { return; }
        if (include_path_it == include_paths.end()) { break; }
        path = *include_path_it++;
    }

    logger::error(expansion_loc).println("could not open input file `{}`", file_name);
}

}  // namespace

DAISY_ADD_PREPROC_DIRECTIVE_PARSER(include, parseIncludeDirective);
