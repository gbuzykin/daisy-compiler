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

void daisy::skipString(TextRange& text) {
    const char *p = text.first, *p0 = p;
    while (p != text.last) {
        switch (*p++) {
            case '\\': {  // Skip any character after '\\', count newlines
                if (p != text.last && *p++ == '\n') { text.pos.nextLn(), p0 = p; }
            } break;
            case '\n': text.pos.nextLn(), p0 = p; break;
            case '\"': goto stop;
        }
    }
stop:
    text.first = p, text.pos.col += static_cast<unsigned>(p - p0);
}

void daisy::skipWhitespaces(TextRange& text) {
    const char *p = text.first, *p0 = p;
    while (p != text.last) {
        switch (*p) {
            case '\r':
            case '\t':
            case ' ': ++p; break;  // Skip whitespace
            case '\\': {           // Treat '\\\n' sequence as whitespace, stop on '\\' otherwise
                if (p + 1 == text.last || *(p + 1) != '\n') { goto stop; }
                text.pos.nextLn(), p0 = p += 2;
            } break;
            case '\n': text.pos.nextLn(), p0 = ++p; break;  // Skip newlines
            default: goto stop;                             // Stop on all other character
        }
    }
stop:
    text.first = p, text.pos.col += static_cast<unsigned>(p - p0);
}

const char* daisy::trimTrailingWhitespaces(const char* first, const char* last) {
    while (last != first) {
        switch (*(last - 1)) {
            case '\r':
            case '\t':
            case ' ': --last; break;  // Skip whitespace
            case '\n': {              // Treat '\\\n' sequence as whitespace
                if (--last == first || *(last - 1) != '\\') { return last; }
                --last;
            } break;
            default: return last;  // Stop on all other character
        }
    }
    return last;
}

void daisy::findMacroArgumentList(TextRange& text) {
    const char *p = text.first, *p0 = p;
    while (p != text.last) {
        switch (*p) {
            case '\r':
            case '\t':
            case ' ': ++p; break;  // Skip whitespace
            case '\\': {           // Treat '\\\n' sequence as whitespace, stop on '\\' otherwise
                if (p + 1 == text.last || *(p + 1) != '\n') { goto stop; }
                text.pos.nextLn(), p0 = p += 2;
            } break;
            case '/': {  // Skip comments
                if (++p != text.last && (*p == '/' || *p == '*')) {
                    text.first = p + 1, text.pos.col += static_cast<unsigned>(p - p0) + 1;
                    if (*p == '/') {
                        skipTillNewLine(text);
                    } else {
                        skipCommentBlock(text);
                    }
                    p = p0 = text.first;
                }
            } break;
            case '\n': text.pos.nextLn(), p0 = ++p; break;  // Skip newlines
            default: goto stop;                             // Stop on all other character
        }
    }
stop:
    text.first = p, text.pos.col += static_cast<unsigned>(p - p0);
}

void daisy::findMacroArgumentSeparator(TextRange& text) {
    const char *p = text.first, *p0 = p;
    unsigned bracket_level = 0;
    while (p != text.last) {
        switch (*p) {
            case '(': ++bracket_level, ++p; break;
            case ')': {  // Stop on closing balanced ')'
                if (bracket_level == 0) { goto stop; }
                --bracket_level, ++p;
            } break;
            case ',': {  // Stop on ',', skip commas in nested brackets
                if (bracket_level == 0) { goto stop; }
                ++p;
            } break;
            case '\\': {  // Skip any character after '\\', count newlines
                if (++p != text.last && *p++ == '\n') { text.pos.nextLn(), p0 = p; }
            } break;
            case '/': {  // Skip comments
                if (++p != text.last && (*p == '/' || *p == '*')) {
                    text.first = p + 1, text.pos.col += static_cast<unsigned>(p - p0) + 1;
                    if (*p == '/') {
                        skipTillNewLine(text);
                    } else {
                        skipCommentBlock(text);
                    }
                    p = p0 = text.first;
                }
            } break;
            case '\"': {  // Skip strings
                text.first = p + 1, text.pos.col += static_cast<unsigned>(p - p0) + 1;
                skipString(text);
                p = p0 = text.first;
            } break;
            case '\n': text.pos.nextLn(), p0 = ++p; break;  // Skip newlines
            default: ++p; break;                            // Skip all other characters
        }
    }
stop:
    text.first = p, text.pos.col += static_cast<unsigned>(p - p0);
}
