#include "ctx/ctx.h"
#include "logger.h"
#include "pass_manager.h"
#include "uxs/algorithm.h"
#include "uxs/cli/parser.h"

#define XSTR(s) STR(s)
#define STR(s)  #s

using namespace daisy;

int main(int argc, char** argv) {
    try {
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
            for (auto const* node = parse_result.node; node; node = node->get_parent()) {
                if (node->get_type() == uxs::cli::node_type::command) {
                    uxs::stdbuf::out.write(static_cast<const uxs::cli::basic_command<char>&>(*node).make_man_page(true));
                    break;
                }
            }
            return 0;
        } else if (show_version) {
            uxs::println(uxs::stdbuf::out, "{}", XSTR(VERSION));
            return 0;
        } else if (parse_result.status != uxs::cli::parsing_status::ok) {
            switch (parse_result.status) {
                case uxs::cli::parsing_status::unknown_option: {
                    logger::fatal().println("unknown command line option `{}`", argv[parse_result.arg_count]);
                } break;
                case uxs::cli::parsing_status::invalid_value: {
                    if (parse_result.arg_count < argc) {
                        logger::fatal().println("invalid command line argument `{}`", argv[parse_result.arg_count]);
                    } else {
                        logger::fatal().println("expected command line argument after `{}`",
                                                argv[parse_result.arg_count - 1]);
                    }
                } break;
                case uxs::cli::parsing_status::unspecified_value: {
                    if (input_file_names.empty()) {
                        logger::fatal().println("no input files specified");
                        return -1;
                    }
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
