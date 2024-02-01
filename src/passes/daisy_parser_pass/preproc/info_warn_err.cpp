#include "../daisy_parser_pass.h"
#include "../text_utils.h"
#include "logger.h"

using namespace daisy;

inline std::string_view parseMessageDirective(DaisyParserPass* pass) {
    auto& in_ctx = pass->getInputContext();
    TextRange msg = in_ctx.text;
    skipWhitespaces(msg);
    msg.last = trimTrailingWhitespaces(msg.first, msg.last);
    return std::string_view(msg.first, msg.last - msg.first);
}

DAISY_ADD_PREPROC_DIRECTIVE_PARSER(info, [](DaisyParserPass* pass, SymbolInfo& tkn) {
    logger::info(tkn.loc).println("{}", parseMessageDirective(pass));
});

DAISY_ADD_PREPROC_DIRECTIVE_PARSER(warning, [](DaisyParserPass* pass, SymbolInfo& tkn) {
    logger::warning(tkn.loc).println("{}", parseMessageDirective(pass));
});

DAISY_ADD_PREPROC_DIRECTIVE_PARSER(error, [](DaisyParserPass* pass, SymbolInfo& tkn) {
    logger::error(tkn.loc).println("{}", parseMessageDirective(pass));
});
