#pragma once

#include "common/const_defs.h"
#include "ctx/ctx.h"
#include "ir/integer_const.h"
#include "pass_manager.h"
#include "uxs/stringcvt.h"

#include <array>
#include <variant>

#define DAISY_ADD_REDUCE_ACTION_HANDLER(act_id, fn) static daisy::ReduceActionHandler g_act_handler_##fn(act_id, fn)
#define DAISY_ADD_PREPROC_DIRECTIVE_PARSER(directive_id, fn, ...) \
    static daisy::PreprocDirectiveParser g_preproc_directive_parser_##fn(directive_id, fn, ##__VA_ARGS__)

namespace lex_detail {
#include "lex_defs.h"
}

namespace parser_detail {
#include "parser_defs.h"
}

namespace daisy {

using SymbolVal = std::variant<bool, IntegerConst, double, std::string_view, std::string>;

struct SymbolInfo {
    SymbolVal val;
    SymbolLoc loc;
};

struct InputContext {
    enum class Flags : unsigned {
        kNone = 0,
        kStopAtEndOfInput = 1,
        kPreprocDirective = 2,
    };
    InputContext(TextRange txt, const LocationContext* ctx) : text(txt), loc_ctx(ctx) {}
    TextRange text;
    const LocationContext* loc_ctx;
    Flags flags = InputContext::Flags::kNone;
};
UXS_IMPLEMENT_BITWISE_OPS_FOR_ENUM(InputContext::Flags, unsigned);

class DaisyParserPass;

struct ReduceActionHandler {
    using FuncType = void (*)(DaisyParserPass*, SymbolInfo*);
    ReduceActionHandler(int id, FuncType fn) : next_avail(first_avail), act_id(id), func(fn) { first_avail = this; }
    static const ReduceActionHandler* first_avail;
    const ReduceActionHandler* next_avail;
    int act_id;
    FuncType func;
};

struct PreprocDirectiveParser {
    using FuncType = void (*)(DaisyParserPass*, SymbolInfo&);
    PreprocDirectiveParser(std::string_view id, FuncType fn) : next_avail(first_avail), directive_id(id), func(fn) {
        first_avail = this;
    }
    static const PreprocDirectiveParser* first_avail;
    const PreprocDirectiveParser* next_avail;
    std::string_view directive_id;
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
    const InputFileInfo* loadInputFile(std::string_view file_path);

    InputContext& getInputContext() { return *input_ctx_stack_.front(); }
    InputContext& pushInputContext(std::unique_ptr<InputContext> in_ctx) {
        return *input_ctx_stack_.emplace_front(std::move(in_ctx));
    }
    bool popInputContext() {
        input_ctx_stack_.pop_front();
        return input_ctx_stack_.empty();
    }

    const LocationContext& newLocationContext(const InputFileInfo* file, const TextExpansion& expansion) {
        return ctx_->loc_ctx_list.emplace_front(file, expansion);
    }

    void ensureEndOfInput(SymbolInfo& tkn);

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
    std::forward_list<std::unique_ptr<InputContext>> input_ctx_stack_;
    uxs::basic_inline_dynbuffer<int, 1> lex_state_stack_;

    std::unordered_map<std::string_view, int> keywords_;
    std::array<ReduceActionHandler::FuncType, parser_detail::total_action_count> reduce_action_handlers_;
    std::unordered_map<std::string_view, const PreprocDirectiveParser*> preproc_directive_parsers_;

    void parsePreprocessorDirective();
};

void logSyntaxError(int tt, const SymbolLoc& loc);

}  // namespace daisy