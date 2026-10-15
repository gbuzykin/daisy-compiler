#include "logger.h"

#include "../daisy_parser_pass.h"

using namespace daisy;

namespace {

void parseIncludeDirective(DaisyParserPass* pass, SymbolInfo& tkn) {
    int tt = pass->lex(tkn);  // Parse reference file path
    if (tt != parser_detail::tt_string_literal) {
        logger::error(tkn.loc).println("expected file name");
        return;
    }

    const std::filesystem::path file_path(uxs::utf_native_path_adapter{}(std::get<std::string>(tkn.val)));
    const auto& include_dirs = pass->getCompilationContext().include_dirs;

    const std::filesystem::path& current_file_path = tkn.loc.loc_ctx->file->file_path;
    const auto parent_path = current_file_path.has_parent_path() ? current_file_path.parent_path() :
                                                                   std::filesystem::path("./");

    const auto* origin_path = &parent_path;

    SymbolLoc expansion_loc = tkn.loc;

    pass->ensureEndOfInput(tkn);

    auto include_dir_it = include_dirs.begin();
    while (true) {
        if (pass->pushInputFile(*origin_path / file_path, expansion_loc)) { return; }
        if (include_dir_it == include_dirs.end()) { break; }
        origin_path = &*include_dir_it++;
    }

    logger::error(expansion_loc).println("could not open input file `{}`", file_path);
}

}  // namespace

DAISY_ADD_PREPROC_DIRECTIVE_PARSER(include, parseIncludeDirective);
