#include "logger.h"
#include "pass_manager.h"

#include "ctx/ctx.h"

#include <uxs/cli/parser.h>

#include <exception>

#define XSTR(s) STR(s)
#define STR(s)  #s

using namespace daisy;

int main(int argc, char** argv) {
    try {
        bool show_help = false;
        bool show_version = false;
        std::vector<std::filesystem::path> input_file_paths;
        std::vector<std::filesystem::path> include_dirs;
        std::vector<std::pair<std::string_view, std::string_view>> macro_defs;

        const auto string_to_path = [](const auto& sval, auto& v) {
            v = uxs::utf_native_path_adapter{}(sval);
            return true;
        };

        const auto add_definition = [](auto def, auto& v) {
            if (!uxs::is_alpha{}(def[0]) && def[0] != '_') { return false; }
            std::string_view val;
            auto pos = def.find('=');
            if (pos != std::string::npos) {
                val = def.substr(pos + 1);
                def = def.substr(0, pos);
            } else {
                val = "1";
            }
            if (!std::ranges::all_of(def.substr(1), [](char ch) { return uxs::is_alnum{}(ch) || ch == '_'; })) {
                return false;
            }
            v = std::make_pair(def, val);
            return true;
        };

        auto cli = uxs::cli::command(argv[0])
                   << uxs::cli::overview("the Daisy compiler")
                   << uxs::cli::values("filename...", input_file_paths, string_to_path)
                   << (uxs::cli::option({"-I"}) & uxs::cli::values("<dir>", include_dirs, string_to_path)) %
                          "Add directory <dir> to the end of the list of include search paths."
                   << (uxs::cli::option({"-D"}) & uxs::cli::values("<macro>={<value>}", macro_defs, add_definition)) %
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
                    if (input_file_paths.empty()) { logger::fatal().println("no input files specified"); }
                } break;
                default: break;
            }
            return -1;
        }

        PassManager::getInstance().configure();

        for (const auto& file_path : input_file_paths) {
            auto ctx = std::make_unique<CompilationContext>(file_path);
            ctx->include_dirs = include_dirs;
            for (const auto& [id, value] : macro_defs) {
                auto macro_def = std::make_unique<MacroDefinition>(MacroDefinition::Type::kUserDefined, id);
                macro_def->text = TextRange{value.data(), value.data() + value.size()};
                ctx->macro_defs[id] = std::move(macro_def);
            }
            PassResult result = PassManager::getInstance().run(*ctx);
            logger::info(uxs::utf_string_adapter<char>{}(file_path.native()))
                .println("warnings {}, errors {}", ctx->warning_count, ctx->error_count);
            if (result != PassResult::kSuccess) { return -1; }
        }

        return 0;

    } catch (const std::exception& e) {
        logger::fatal().println("exception caught: {}", e.what());
        return -1;
    }
}
