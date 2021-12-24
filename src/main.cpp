#include "ctx/ctx.h"
#include "logger.h"
#include "pass_manager.h"

using namespace daisy;

int main(int argc, char** argv) {
    try {
        std::vector<std::string> input_file_names;
        std::vector<std::string_view> include_paths;
        std::vector<std::pair<std::string_view, std::string_view>> macro_defs;
        for (int i = 1; i < argc; ++i) {
            std::string_view arg(argv[i]);
            if (arg == "--help") {
                // clang-format off
                static std::string_view text[] = {
                    "Usage: daisy-compiler [options] file...",
                    "Options:",
                    "    -I<path>       Add path to search files to include.",
                    "    -D<id>{=...}   Add macro definition.",
                    "    -d<N>          Debug verbosity level.",
                    "    --help         Display this information.",
                };
                // clang-format on
                for (const auto& l : text) { uxs::println("{}", l); }
                return 0;
            } else if (arg[0] != '-') {
                input_file_names.emplace_back(arg);
            } else if (arg[1] == 'd') {
                logger::g_debug_level = arg.size() > 2 ? arg[2] - '0' : 2;
            } else if (arg.size() > 1 && arg[1] == 'I') {
                if (arg.size() > 2) { include_paths.emplace_back(arg.substr(2)); }
            } else if (arg.size() > 1 && arg[1] == 'D') {
                auto it = arg.begin() + 2;
                if (it == arg.end() || (!uxs::is_alpha(*it) && *it != '_')) {
                    logger::fatal().format("argument to `-D` is invalid");
                    return -1;
                }
                while (it != arg.end() && (uxs::is_alnum(*it) || *it == '_')) { ++it; }

                std::string_view id = arg.substr(2, it - arg.begin() - 2), value;
                if (it != arg.end() && *it++ == '=') { value = arg.substr(it - arg.begin(), arg.end() - it); }
                macro_defs.emplace_back(id, value);
            } else {
                logger::fatal().format("unknown flag `{}`", arg);
                return -1;
            }
        }

        if (input_file_names.empty()) {
            logger::fatal().format("no input files specified");
            return -1;
        }

        PassManager::getInstance().configure();

        for (const auto& file_name : input_file_names) {
            auto ctx = std::make_unique<CompilationContext>(file_name);
            ctx->include_paths = include_paths;
            for (const auto& [id, value] : macro_defs) {
                auto& macro_def = ctx->macro_defs[id];
                macro_def = MacroDefinition{MacroDefinition::Type::kUserDefined, id};
                macro_def.text = TextRange{value.data(), value.data() + value.size()};
            }
            PassResult result = PassManager::getInstance().run(*ctx);
            logger::info(file_name).format("warnings {}, errors {}", ctx->warning_count, ctx->error_count);
            if (result != PassResult::kSuccess) { return -1; }
        }

        return 0;
    } catch (const std::exception& e) { logger::fatal().format("exception caught: {}", e.what()); }
    return -1;
}
