#include "text_utils.h"

#include <algorithm>

using namespace daisy;

void daisy::skipTillNewLine(TextRange& text) {
    const char* eol = std::find(text.first, text.last, '\n');
    if (eol != text.first) {
        text.pos.col += static_cast<unsigned>(eol - text.first);
        while (eol != text.last && *(eol - 1) == '\\') {
            const char* next_eol = std::find(eol + 1, text.last, '\n');
            ++text.pos.ln, text.pos.col = static_cast<unsigned>(next_eol - eol);
            eol = next_eol;
        }
    }
    text.first = eol;
}

bool daisy::skipCommentBlock(TextRange& text) {
    const char *p = text.first, *p0 = p;
    bool is_terminated = false;
    if (p != text.last) {
        if (*p == '/') { ++p; }
        while (p != text.last) {
            if (char ch = *p++; ch == '\n') {
                text.pos.nextLn(), p0 = p;
            } else if (ch == '/' && *(p - 2) == '*') {
                is_terminated = true;
                break;
            }
        }
    }
    text.first = p, text.pos.col += static_cast<unsigned>(p - p0);
    return is_terminated;
}
