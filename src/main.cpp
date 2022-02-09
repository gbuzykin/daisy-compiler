#include "ctx/ctx.h"
#include "logger.h"
#include "pass_manager.h"

#include "uxs/cli/parser.h"

#include <uxs/algorithm.h>

#include <exception>

//-----------------------------------------
#include "ir/nodes/bool_const_node.h"
#include "ir/nodes/float_const_node.h"
#include "ir/nodes/int_const_node.h"
#include "ir/nodes/op_node.h"
#include "ir/nodes/string_const_node.h"

#include <chrono>
#include <random>
//-----------------------------------------

#define XSTR(s) STR(s)
#define STR(s)  #s

using namespace daisy;

int rtti_test() {
    {
        std::unique_ptr<ir::Node> obj = std::make_unique<ir::BoolConstNode>(false, SymbolLoc{});
        if (!util::is_kind_of<ir::Node>(*obj)) { throw std::runtime_error("rtti error"); }
        if (!util::is_kind_of<ir::EvalNode>(*obj)) { throw std::runtime_error("rtti error"); }
        if (!util::is_kind_of<ir::BoolConstNode>(*obj)) { throw std::runtime_error("rtti error"); }
        if (util::is_kind_of<ir::FloatConstNode>(*obj)) { throw std::runtime_error("rtti error"); }
    }

    {
        std::unique_ptr<ir::EvalNode> obj = std::make_unique<ir::BoolConstNode>(false, SymbolLoc{});
        if (!util::is_kind_of<ir::Node>(*obj)) { throw std::runtime_error("rtti error"); }
        if (!util::is_kind_of<ir::EvalNode>(*obj)) { throw std::runtime_error("rtti error"); }
        if (!util::is_kind_of<ir::BoolConstNode>(*obj)) { throw std::runtime_error("rtti error"); }
        if (util::is_kind_of<ir::FloatConstNode>(*obj)) { throw std::runtime_error("rtti error"); }
    }

    {
        std::unique_ptr<ir::BoolConstNode> obj = std::make_unique<ir::BoolConstNode>(false, SymbolLoc{});
        if (!util::is_kind_of<ir::Node>(*obj)) { throw std::runtime_error("rtti error"); }
        if (!util::is_kind_of<ir::EvalNode>(*obj)) { throw std::runtime_error("rtti error"); }
        if (!util::is_kind_of<ir::BoolConstNode>(*obj)) { throw std::runtime_error("rtti error"); }
        if (util::is_kind_of<ir::FloatConstNode>(*obj)) { throw std::runtime_error("rtti error"); }
    }

    {
        std::unique_ptr<ir::Node> obj = std::make_unique<ir::EvalNode>(SymbolLoc{});
        if (!util::is_kind_of<ir::Node>(*obj)) { throw std::runtime_error("rtti error"); }
        if (!util::is_kind_of<ir::EvalNode>(*obj)) { throw std::runtime_error("rtti error"); }
        if (util::is_kind_of<ir::BoolConstNode>(*obj)) { throw std::runtime_error("rtti error"); }
        if (util::is_kind_of<ir::FloatConstNode>(*obj)) { throw std::runtime_error("rtti error"); }
    }

    {
        std::unique_ptr<ir::EvalNode> obj = std::make_unique<ir::EvalNode>(SymbolLoc{});
        if (!util::is_kind_of<ir::Node>(*obj)) { throw std::runtime_error("rtti error"); }
        if (!util::is_kind_of<ir::EvalNode>(*obj)) { throw std::runtime_error("rtti error"); }
        if (util::is_kind_of<ir::BoolConstNode>(*obj)) { throw std::runtime_error("rtti error"); }
        if (util::is_kind_of<ir::FloatConstNode>(*obj)) { throw std::runtime_error("rtti error"); }
    }

    {
        std::unique_ptr<ir::Node> obj = std::make_unique<ir::Node>(SymbolLoc{});
        if (!util::is_kind_of<ir::Node>(*obj)) { throw std::runtime_error("rtti error"); }
        if (util::is_kind_of<ir::EvalNode>(*obj)) { throw std::runtime_error("rtti error"); }
        if (util::is_kind_of<ir::BoolConstNode>(*obj)) { throw std::runtime_error("rtti error"); }
        if (util::is_kind_of<ir::FloatConstNode>(*obj)) { throw std::runtime_error("rtti error"); }
    }

    std::default_random_engine gen;
    std::uniform_int_distribution<int> dist(0, 3);
    std::vector<std::unique_ptr<ir::Node>> v;

    {
        auto start = std::chrono::high_resolution_clock::now();
        const unsigned N = 10000000;
        for (unsigned n = 0; n < N; ++n) {
            switch (dist(gen)) {
                case 0: {
                    v.emplace_back(std::make_unique<ir::BoolConstNode>(false, SymbolLoc{}));
                } break;
                case 1: {
                    v.emplace_back(std::make_unique<ir::FloatConstNode>(ir::FloatConst{3.1415}, SymbolLoc{}));
                } break;
                case 2: {
                    v.emplace_back(std::make_unique<ir::IntConstNode>(ir::IntConst::fromUInt64(ir::IntType::i32, 100),
                                                                      SymbolLoc{}));
                } break;
                case 3: {
                    v.emplace_back(std::make_unique<ir::StringConstNode>(std::string{"hello"}, SymbolLoc{}));
                } break;
            }
        }
        std::chrono::duration<double> diff = std::chrono::high_resolution_clock::now() - start;
        uxs::println("init time: {} s", diff.count());
    }

    {
        auto start = std::chrono::high_resolution_clock::now();
        unsigned cnt[4] = {0, 0, 0, 0};
        for (const auto& p : v) {
            if (util::is_kind_of<ir::BoolConstNode>(*p)) {
                ++cnt[0];
            } else if (util::is_kind_of<ir::FloatConstNode>(*p)) {
                ++cnt[1];
            } else if (util::is_kind_of<ir::IntConstNode>(*p)) {
                ++cnt[2];
            } else if (util::is_kind_of<ir::StringConstNode>(*p)) {
                ++cnt[3];
            }
        }
        std::chrono::duration<double> diff = std::chrono::high_resolution_clock::now() - start;
        uxs::println("rtti time: {} s ({}, {}, {}, {})", diff.count(), cnt[0], cnt[1], cnt[2], cnt[3]);
    }

    {
        auto start = std::chrono::high_resolution_clock::now();
        unsigned cnt[4] = {0, 0, 0, 0};
        for (const auto& p : v) {
            if (dynamic_cast<ir::BoolConstNode*>(p.get())) {
                ++cnt[0];
            } else if (dynamic_cast<ir::FloatConstNode*>(p.get())) {
                ++cnt[1];
            } else if (dynamic_cast<ir::IntConstNode*>(p.get())) {
                ++cnt[2];
            } else if (dynamic_cast<ir::StringConstNode*>(p.get())) {
                ++cnt[3];
            }
        }
        std::chrono::duration<double> diff = std::chrono::high_resolution_clock::now() - start;
        uxs::println("std rtti time: {} s ({}, {}, {}, {})", diff.count(), cnt[0], cnt[1], cnt[2], cnt[3]);
    }
    return 0;
}

int main(int argc, char** argv) {
    try {
        rtti_test();
        return 0;

        bool show_help = false, show_version = false;
        std::vector<std::string> input_file_names;
        std::vector<std::string_view> include_paths;
        std::vector<std::pair<std::string_view, std::string_view>> macro_defs;

        auto add_definition = [&macro_defs](std::string_view def) {
            if (!uxs::is_alpha(def[0]) && def[0] != '_') { return false; }
            std::string_view val;
            auto pos = def.find('=');
            if (pos != std::string::npos) {
                val = def.substr(pos + 1);
                def = def.substr(0, pos);
            } else {
                val = "1";
            }
            if (!uxs::all_of(def.substr(1), [](char ch) { return uxs::is_alnum(ch) || ch == '_'; })) { return false; }
            macro_defs.emplace_back(std::make_pair(def, val));
            return true;
        };

        auto cli = uxs::cli::command(argv[0])
                   << uxs::cli::overview("the Daisy compiler") << uxs::cli::values("filename...", input_file_names)
                   << (uxs::cli::option({"-I"}) &
                       uxs::cli::basic_value_wrapper<char>("<dir>",
                                                           [&include_paths](std::string_view path) {
                                                               include_paths.emplace_back(path);
                                                               return true;
                                                           })) %
                          "Add directory <dir> to the end of the list of include search paths."
                   << (uxs::cli::option({"-D"}) &
                       uxs::cli::basic_value_wrapper<char>("<macro>={<value>}", add_definition)) %
                          "Define <macro> to <value> (or 1 if <value> omitted)."
                   << (uxs::cli::option({"-d", "--debug-level="}) & uxs::cli::value("<n>", logger::g_debug_level)) %
                          "Debug verbosity level."
                   << uxs::cli::option({"-h", "--help"}).set(show_help) % "Display this information."
                   << uxs::cli::option({"-V", "--version"}).set(show_version) % "Display version.";

        auto parse_result = cli->parse(argc, argv);
        if (show_help) {
            uxs::stdbuf::out().write(parse_result.node->get_command()->make_man_page(uxs::cli::text_coloring::colored));
            return 0;
        } else if (show_version) {
            uxs::println(uxs::stdbuf::out(), "{}", XSTR(VERSION));
            return 0;
        } else if (parse_result.status != uxs::cli::parsing_status::ok) {
            switch (parse_result.status) {
                case uxs::cli::parsing_status::unknown_option: {
                    logger::fatal().println("unknown command line option `{}`", argv[parse_result.argc_parsed]);
                } break;
                case uxs::cli::parsing_status::invalid_value: {
                    if (parse_result.argc_parsed < argc) {
                        logger::fatal().println("invalid command line argument `{}`", argv[parse_result.argc_parsed]);
                    } else {
                        logger::fatal().println("expected command line argument after `{}`",
                                                argv[parse_result.argc_parsed - 1]);
                    }
                } break;
                case uxs::cli::parsing_status::unspecified_value: {
                    if (input_file_names.empty()) { logger::fatal().println("no input files specified"); }
                } break;
                default: break;
            }
            return -1;
        }

        PassManager::getInstance().configure();

        for (const auto& file_name : input_file_names) {
            auto ctx = std::make_unique<CompilationContext>(file_name);
            ctx->include_paths = include_paths;
            for (const auto& [id, value] : macro_defs) {
                auto macro_def = std::make_unique<MacroDefinition>(MacroDefinition::Type::kUserDefined, id);
                macro_def->text = TextRange{value.data(), value.data() + value.size()};
                ctx->macro_defs[id] = std::move(macro_def);
            }
            PassResult result = PassManager::getInstance().run(*ctx);
            logger::info(file_name).println("warnings {}, errors {}", ctx->warning_count, ctx->error_count);
            if (result != PassResult::kSuccess) { return -1; }
        }

        return 0;
    } catch (const std::exception& e) { logger::fatal().println("exception caught: {}", e.what()); }
    return -1;
}
