#pragma once

#include "common/symbol_loc.h"

namespace daisy {

void skipTillNewLine(TextRange& text);
bool skipCommentBlock(TextRange& text);

}  // namespace daisy
