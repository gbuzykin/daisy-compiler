#include "../daisy_parser_pass.h"
#include "ctx/ctx.h"
#include "logger.h"

#include <filesystem>

using namespace daisy;

namespace {

void parseIncludeDirective(DaisyParserPass* pass, SymbolInfo& tkn) {
    int tt = pass->lex(tkn);  // Parse reference file path
    if (tt != parser_detail::tt_string_literal) {
        logger::error(tkn.loc).format("expected file name");
        return;
    }

    const auto file_name = std::move(std::get<std::string>(tkn.val));
    const auto& include_paths = pass->getCompilationContext().include_paths;

    std::filesystem::path path(tkn.loc.loc_ctx->file->file_name);
    path = path.has_parent_path() ? path.parent_path() : "./";
    auto include_path_it = include_paths.begin();
    while (true) {
        path /= file_name;

        // Load input file to include
        if (const auto* input_file = pass->loadInputFile(path.generic_string()); input_file) {
            SymbolLoc expansion_loc = tkn.loc;
            pass->ensureEndOfInput(tkn);
            pass->pushInputContext(std::make_unique<InputContext>(
                input_file->getText(), &pass->newLocationContext(input_file, TextExpansion{expansion_loc})));
            return;
        }

        if (include_path_it == include_paths.end()) { break; }
        path = *include_path_it++;
    }

    logger::error(tkn.loc).format("could not open input file `{}`", file_name);
}

}  // namespace

DAISY_ADD_PREPROC_DIRECTIVE_PARSER("include", parseIncludeDirective);
