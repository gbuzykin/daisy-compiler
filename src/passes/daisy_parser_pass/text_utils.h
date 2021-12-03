#pragma once

#include "common/symbol_loc.h"

namespace daisy {

void skipTillNewLine(TextRange& text);
bool skipCommentBlock(TextRange& text);
void skipString(TextRange& text);
void skipWhitespaces(TextRange& text);
const char* trimTrailingWhitespaces(const char* first, const char* last);
void findMacroArgumentList(TextRange& text);
void findMacroArgumentSeparator(TextRange& text);
void skipTillPreprocDirective(TextRange& text);

}  // namespace daisy
