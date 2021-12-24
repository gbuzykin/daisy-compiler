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

void parseInfoDirective(DaisyParserPass* pass, SymbolInfo& tkn) {
    logger::info(tkn.loc).format("{}", parseMessageDirective(pass));
}

void parseWarningDirective(DaisyParserPass* pass, SymbolInfo& tkn) {
    logger::warning(tkn.loc).format("{}", parseMessageDirective(pass));
}

void parseErrorDirective(DaisyParserPass* pass, SymbolInfo& tkn) {
    logger::error(tkn.loc).format("{}", parseMessageDirective(pass));
}

DAISY_ADD_PREPROC_DIRECTIVE_PARSER("info", parseInfoDirective);
DAISY_ADD_PREPROC_DIRECTIVE_PARSER("warning", parseWarningDirective);
DAISY_ADD_PREPROC_DIRECTIVE_PARSER("error", parseErrorDirective);
