#pragma once

#include "common/const_defs.h"
#include "ctx/ctx.h"
#include "ir/integer_const.h"
#include "pass_manager.h"
#include "uxs/stringcvt.h"

#include <array>
#include <forward_list>
#include <unordered_map>
#include <variant>

#define DAISY_ADD_REDUCE_ACTION_HANDLER(act_id, fn) static daisy::ReduceActionHandler g_act_handler_##fn(act_id, fn)

namespace lex_detail {
#include "lex_defs.h"
}

namespace parser_detail {
#include "parser_defs.h"
}

namespace daisy {

using SymbolVal = std::variant<bool, IntegerConst, double, std::string>;

struct SymbolInfo {
    SymbolVal val;
    SymbolLoc loc;
};

struct InputContext {
    InputContext(const TextRange& t, const LocationContext* ctx) : text(t), loc_ctx(ctx) {}
    TextRange text;
    const LocationContext* loc_ctx;
};

class DaisyParserPass;

struct ReduceActionHandler {
    using FuncType = void (*)(DaisyParserPass*, SymbolInfo*);
    ReduceActionHandler(int id, FuncType fn) : next_avail(first_avail), act_id(id), func(fn) { first_avail = this; }
    static const ReduceActionHandler* first_avail;
    const ReduceActionHandler* next_avail;
    int act_id;
    FuncType func;
};

class DaisyParserPass : public Pass {
 public:
    std::string_view getName() const override { return "DaisyParserPass"; }
    void configure() override;
    PassResult run(CompilationContext& ctx) override;
    void cleanup() override;

    CompilationContext& getCompilationContext() const { return *ctx_; }
    int lex(SymbolInfo& tkn);
    const InputFileInfo* loadInputFile(const std::string& file_name);

    InputContext* getInputContext() { return &input_ctx_stack_.front(); }
    InputContext* pushInputContext(const TextRange& text, const LocationContext* ctx) {
        return &input_ctx_stack_.emplace_front(text, ctx);
    }
    bool popInputContext() {
        input_ctx_stack_.pop_front();
        return input_ctx_stack_.empty();
    }

    const LocationContext* makeNewLocContext(const InputFileInfo* file, TextExpansion::Type exp_type,
                                             const SymbolLoc& exp_loc) {
        return &ctx_->loc_ctx_list.emplace_front(file, exp_type, exp_loc);
    }

 private:
    struct TextBuffer {
        explicit TextBuffer(size_t sz) : text(std::make_unique<char[]>(sz)), text_last(text.get() + sz) {}
        size_t getSize() const { return text_last - text.get(); }
        std::unique_ptr<char[]> text;
        char* text_last;
    };

    CompilationContext* ctx_ = nullptr;
    unsigned error_status_ = 0;

    SymbolInfo la_tkn_;
    std::forward_list<InputContext> input_ctx_stack_;
    uxs::basic_inline_dynbuffer<int, 1> lex_state_stack_;

    std::unordered_map<std::string_view, int> keywords_;
    std::array<ReduceActionHandler::FuncType, parser_detail::total_action_count> reduce_action_handlers_;
};

void logSyntaxError(int tt, const SymbolLoc& loc);

}  // namespace daisy
