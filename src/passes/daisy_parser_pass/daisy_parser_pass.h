#pragma once

#include "common/const_defs.h"
#include "ctx/ctx.h"
#include "ir/float_const.h"
#include "ir/int_const.h"
#include "ir/scope_descriptor.h"
#include "ir/type_descriptor.h"
#include "pass_manager.h"
#include "uxs/stringcvt.h"

#include <array>
#include <variant>

#define DAISY_ADD_REDUCE_ACTION_HANDLER(act_id, fn) \
    static daisy::ReduceActionHandler g_act_handler_##act_id(parser_detail::act_id, fn)
#define DAISY_ADD_PREPROC_DIRECTIVE_PARSER(directive_id, fn, ...) \
    static daisy::PreprocDirectiveParser g_preproc_directive_parser_##directive_id(#directive_id, fn, ##__VA_ARGS__)

namespace lex_detail {
#include "lex_defs.h"
}

namespace parser_detail {
#include "parser_defs.h"
}

namespace daisy {

using SymbolVal =
    std::variant<bool, ir::IntConst, ir::FloatConst, std::string, std::string_view, std::unique_ptr<ir::Node>,
                 ir::ScopeDescriptor, ir::TypeDescriptor, ir::DataTypeModifiers>;

struct SymbolInfo {
    SymbolVal val;
    SymbolLoc loc;
};

struct MacroExpansion;

struct IfSectionState {
    SymbolLoc loc;
    bool is_matched = false;
    bool has_else_section = false;
    unsigned section_disable_counter = 0;
};

struct InputContext {
    enum class Flags : unsigned {
        kNone = 0,
        kStopAtEndOfInput = 1,
        kPreprocDirective = 2,
        kExpendingMacro = 4,
        kDisableMacroExpansion = 8,
        kSkipFile = 0x10,
    };
    InputContext(TextRange txt, const LocationContext* ctx, Flags f = Flags::kNone)
        : text(txt), loc_ctx(ctx), flags(f) {}
    virtual ~InputContext() = default;
    TextRange text;
    const LocationContext* loc_ctx;
    Flags flags;
    MacroExpansion* macro_expansion = nullptr;
    const IfSectionState* last_if_section_state = nullptr;
};
UXS_IMPLEMENT_BITWISE_OPS_FOR_ENUM(InputContext::Flags, unsigned);

struct MacroExpansion {
    const MacroDefinition* macro_def;
    const InputContext* source_in_ctx;
    SymbolLoc loc;
    std::vector<TextRange> actual_args;
};

struct MacroExpansionContext : public InputContext {
    template<typename... Args>
    MacroExpansionContext(TextRange txt, const LocationContext* ctx, Args&&... macro_exp_args)
        : InputContext(txt, ctx, Flags::kExpendingMacro), macro_expansion_info{std::forward<Args>(macro_exp_args)...} {}
    MacroExpansion macro_expansion_info;
};

class DaisyParserPass;

struct ReduceActionHandler {
    using FuncType = void (*)(DaisyParserPass*, SymbolInfo*, SymbolLoc& loc);
    ReduceActionHandler(int id, FuncType fn) : next_avail(first_avail), act_id(id), func(fn) { first_avail = this; }
    static const ReduceActionHandler* first_avail;
    const ReduceActionHandler* next_avail;
    int act_id;
    FuncType func;
};

struct PreprocDirectiveParser {
    using FuncType = void (*)(DaisyParserPass*, SymbolInfo&);
    PreprocDirectiveParser(std::string_view id, FuncType fn, bool pdt = false)
        : next_avail(first_avail), directive_id(id), func(fn), parse_disabled_text(pdt) {
        first_avail = this;
    }
    static const PreprocDirectiveParser* first_avail;
    const PreprocDirectiveParser* next_avail;
    std::string_view directive_id;
    FuncType func;
    bool parse_disabled_text;
};

class DaisyParserPass : public Pass {
 public:
    std::string_view getName() const override { return "DaisyParserPass"; }
    void configure() override;
    PassResult run(CompilationContext& ctx) override;
    void cleanup() override;

    CompilationContext& getCompilationContext() const { return *ctx_; }
    int lex(SymbolInfo& tkn, bool* leading_ws = nullptr);
    static int parse(int tt, int* sptr0, int** p_sptr, int rise_error);
    const InputFileInfo* pushInputFile(std::string_view file_path, const SymbolLoc& expansion_loc);
    bool isKeyword(std::string_view id) const;

    IfSectionState* getIfSection() { return !if_section_stack_.empty() ? &if_section_stack_.front() : nullptr; }
    IfSectionState& pushIfSection(const SymbolLoc& loc) { return if_section_stack_.emplace_front(IfSectionState{loc}); }
    void popIfSection() { if_section_stack_.pop_front(); }

    InputContext& getInputContext() { return *input_ctx_stack_.front(); }
    InputContext& pushInputContext(std::unique_ptr<InputContext> in_ctx) {
        in_ctx->last_if_section_state = getIfSection();
        return *input_ctx_stack_.emplace_front(std::move(in_ctx));
    }
    bool popInputContext() {
        input_ctx_stack_.pop_front();
        return input_ctx_stack_.empty();
    }

    template<typename... Args>
    LocationContext& newLocationContext(const InputFileInfo* file, Args&&... expansion_args) {
        return ctx_->loc_ctx_list.emplace_front(file, std::forward<Args>(expansion_args)...);
    }

    InputContext& pushStringInputContext(std::string str, const MacroExpansion& macro_exp) {
        auto& text = ctx_->input_strings.emplace_front(std::move(str));
        return pushInputContext(
            std::make_unique<InputContext>(TextRange{text.data(), text.data() + text.size()},
                                           &newLocationContext(nullptr, macro_exp.loc, macro_exp.macro_def)));
    }

    bool checkMacroExpansionForRecursion(std::string_view macro_id);
    void ensureEndOfInput(SymbolInfo& tkn);

    const ir::RootNode& getRootScope() const {
        assert(ctx_->ir_root);
        return *ctx_->ir_root;
    }
    ir::RootNode& getRootScope() {
        assert(ctx_->ir_root);
        return *ctx_->ir_root;
    }

    const ir::Node& getCurrentScope() const {
        assert(current_scope_);
        return *current_scope_;
    }
    ir::Node& getCurrentScope() {
        assert(current_scope_);
        return *current_scope_;
    }
    void setCurrentScope(ir::Node& scope) { current_scope_ = &scope; }
    void popCurrentScope() {
        current_scope_ = current_scope_->getNamespace().getParentScope();
        assert(current_scope_);
    }

 private:
    struct TextBuffer {
        explicit TextBuffer(size_t sz) : text(std::make_unique<char[]>(sz)), text_last(text.get() + sz) {}
        size_t getSize() const { return text_last - text.get(); }
        std::unique_ptr<char[]> text;
        char* text_last;
    };

    CompilationContext* ctx_ = nullptr;
    int at_beginning_of_line_ = 0;
    unsigned error_status_ = 0;

    SymbolInfo la_tkn_;
    std::forward_list<std::unique_ptr<InputContext>> input_ctx_stack_;
    uxs::inline_basic_dynbuffer<int, 1> lex_state_stack_;
    std::forward_list<IfSectionState> if_section_stack_;

    ir::Node* current_scope_;

    std::array<ReduceActionHandler::FuncType, parser_detail::total_action_count> reduce_action_handlers_;
    std::unordered_map<std::string_view, const PreprocDirectiveParser*> preproc_directive_parsers_;

    void parsePreprocessorDirective();
    void defineBuiltinMacros();
    void expandMacro(const SymbolLoc& loc, const MacroDefinition& macro_def);
    void expandMacroArgument(const TextRange& arg);
};

void logSyntaxError(int tt, const SymbolLoc& loc);

}  // namespace daisy
