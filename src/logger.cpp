#include "logger.h"

#include "ctx/ctx.h"
#include "uxs/utf.h"

unsigned daisy::logger::g_debug_level = 1;

using namespace daisy;
using namespace daisy::logger;

namespace {

std::pair<std::string, std::string> markInputLine(std::string_view line, unsigned first, unsigned last) {
    // Note: `first` - left marking boundary, starts from 1; value 0 - no boundary
    // Note: `last` - right marking inclusive boundary, starts from 1; value 0 - no boundary
    assert(first == 0 || last == 0 || first <= last);
    assert(first <= line.size() + 1 && last <= line.size() + 1);

    // If a boundary is not specified use position of the first not space character

    auto p_from = line.begin();
    if (first == 0) {
        while (p_from != line.end() && uxs::is_space(*p_from)) { ++p_from; }
    } else {
        p_from += first - 1;
    }

    auto p_to = line.end();
    if (p_from != line.end()) {
        if (last == 0) {
            while (--p_to != p_from && uxs::is_space(*p_to)) {}
        } else {
            p_to = std::max(p_from, line.begin() + last - 1);
        }
    } else if (first == 0) {
        return {"", ""};
    }

    std::string tab2space_line;
    tab2space_line.reserve(256);

    const unsigned tab_size = 4;
    unsigned col = 0, mark_limits[2] = {0, 0};
    std::uint32_t code = 0;
    for (auto p = line.begin(), next = p; uxs::from_utf8(p, line.end(), next, code) != 0; p = next) {
        if (code == '\t') {  // Convert tab into spaces
            auto align_up = [](unsigned v, unsigned base) { return (v + base - 1) & ~(base - 1); };
            const unsigned tab_pos = align_up(col + 1, tab_size);
            while (col < tab_pos) { tab2space_line.push_back(' '), ++col; }
        } else if (uxs::is_space(code)) {
            tab2space_line.push_back(' '), ++col;
        } else {
            do { tab2space_line.push_back(*p++); } while (p != next);
            ++col;
        }
        if (next <= p_from) { mark_limits[0] = col; }
        if (next <= p_to) { mark_limits[1] = col; }
    }

    std::string mark;
    mark.reserve(256);
    mark.append(mark_limits[0], ' ');
    mark += first == 0 ? '~' : '^';
    mark.append(mark_limits[1] - mark_limits[0], '~');
    return {tab2space_line, mark};
}

std::string_view typeString(MsgType type) {
    switch (type) {
        case MsgType::kDebug: return ": \033[1;33mdebug: \033[0m";
        case MsgType::kInfo: return ": \033[0;34minfo: \033[0m";
        case MsgType::kNote: return ": \033[0;36mnote: \033[0m";
        case MsgType::kWarning: return ": \033[0;35mwarning: \033[0m";
        case MsgType::kError: return ": \033[0;31merror: \033[0m";
        case MsgType::kFatal: return ": \033[0;31mfatal error: \033[0m";
    }
}

void printMessageImpl(MsgType type, const SymbolLoc& loc, std::string_view msg) {
    const auto* file = loc.loc_ctx->file;
    assert(file);

    std::string n_line = uxs::to_string(loc.first.ln);
    uxs::println(uxs::stdbuf::log, "\033[1;37m{}:{}:{}{}{}", file->file_name, n_line, loc.first.col, typeString(type),
                 msg);

    std::string left_padding(n_line.size(), ' ');
    const auto& text_lines = file->text_lines;

    for (unsigned ln = loc.first.ln; ln <= loc.last.ln; ++ln) {
        // Note: line and column numbers start from 1
        auto [tab2space_line, mark] = markInputLine(text_lines[ln - 1], ln == loc.first.ln ? loc.first.col : 0,
                                                    ln == loc.last.ln ? loc.last.col : 0);
        uxs::println(uxs::stdbuf::log, " {} | {}", ln == loc.first.ln ? n_line : left_padding, tab2space_line);
        uxs::println(uxs::stdbuf::log, " {} | \033[0;32m{}\033[0m", left_padding, mark);
    }
}

}  // namespace

LoggerSimple& LoggerSimple::show() {
    if (getType() >= MsgType::kInfo + g_debug_level) { return *this; }
    uxs::println(uxs::stdbuf::log, "\033[1;37m{}{}{}", header_, typeString(getType()), getMessage());
    clear();
    return *this;
}

LoggerExtended& LoggerExtended::show() {
    if (getType() >= MsgType::kInfo + g_debug_level) { return *this; }

    // Unwind location stack
    std::vector<const SymbolLoc*> loc_stack;
    loc_stack.reserve(64);
    for (const auto* loc = &loc_; loc->loc_ctx; loc = &loc->loc_ctx->expansion.loc) { loc_stack.push_back(loc); }

    assert(!loc_stack.empty());
    const auto* loc_ctx = loc_stack.back()->loc_ctx;
    assert(loc_ctx->file && loc_ctx->file->compilation_ctx);
    switch (getType()) {
        case MsgType::kWarning: ++loc_ctx->file->compilation_ctx->warning_count; break;
        case MsgType::kError: ++loc_ctx->file->compilation_ctx->error_count; break;
        case MsgType::kFatal: ++loc_ctx->file->compilation_ctx->error_count; break;
        default: break;
    }

    // Print include history for first
    auto it = loc_stack.rbegin();
    while (it != loc_stack.rend() - 1 && !(*(it + 1))->loc_ctx->expansion.macro_def) {
        assert((*it)->loc_ctx->file);
        if (print_ext_loc_info_) {
            uxs::println(uxs::stdbuf::log, "In file included from {}:{}", (*it)->loc_ctx->file->file_name,
                         (*it)->first.ln);
        }
        ++it;
    }

    // Print main message
    printMessageImpl(getType(), **it, getMessage());

    if (print_ext_loc_info_) {
        // Print macro expansion sequence
        while (++it != loc_stack.rend()) {
            const auto* loc_ctx = (*it)->loc_ctx;
            if (!loc_ctx->file) { break; }
            if (const auto* macro_def = loc_ctx->expansion.macro_def; macro_def) {
                printMessageImpl(MsgType::kNote, **it, uxs::format("expanded from macro `{}`", macro_def->id));
            }
        }
    }

    clear();
    return *this;
}
