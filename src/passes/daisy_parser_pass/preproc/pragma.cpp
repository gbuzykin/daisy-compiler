#include "../daisy_parser_pass.h"
#include "logger.h"

using namespace daisy;

namespace {

void pragmaOnce(DaisyParserPass* pass, SymbolInfo& tkn) {
    auto& in_ctx = pass->getInputContext();
    assert(in_ctx.loc_ctx->file);
    if (!!(in_ctx.loc_ctx->file->flags & InputFileInfo::Flags::kOnce)) {
        in_ctx.flags |= InputContext::Flags::kSkipFile;
    }
    in_ctx.loc_ctx->file->flags |= InputFileInfo::Flags::kOnce;
}

using PragmaImpl = void (*)(DaisyParserPass*, SymbolInfo& tkn);
const std::unordered_map<std::string_view, PragmaImpl> g_pragma_impl = {
    // <id, impl_func>
    {"once", pragmaOnce},
};

}  // namespace

void parsePragmaDirective(DaisyParserPass* pass, SymbolInfo& tkn) {
    int tt = pass->lex(tkn);  // Parse pragma name
    if (tt != parser_detail::tt_id) {
        logger::error(tkn.loc).format("expected pragma name");
        return;
    }

    auto it = g_pragma_impl.find(std::get<std::string_view>(tkn.val));
    if (it != g_pragma_impl.end()) {
        it->second(pass, tkn);
    } else {
        logger::warning(tkn.loc).format("unknown pragma name");
    }
}

DAISY_ADD_PREPROC_DIRECTIVE_PARSER("pragma", parsePragmaDirective);
