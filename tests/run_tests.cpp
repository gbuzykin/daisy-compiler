#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

bool getTestList(const std::string& path, const std::vector<std::string>& subdir_lst, std::vector<std::string>& lst) {
    if (!std::filesystem::exists(path)) {
        std::cerr << "path \'" << path << "\' does not exist" << std::endl;
        return false;
    }
    std::filesystem::directory_iterator end_itr{};
    for (std::filesystem::directory_iterator itr(path); itr != end_itr; ++itr) {
        const auto& name = itr->path().string();
        if (itr->path().filename().c_str()[0] == '_') { continue; }
        if (std::filesystem::is_directory(itr->status())) {
            if (!subdir_lst.empty() && std::find(subdir_lst.begin(), subdir_lst.end(), name) == subdir_lst.end()) {
                continue;
            }
            if (!getTestList(name, {}, lst)) { return false; }
        } else if (itr->path().extension() == ".ds") {
            lst.emplace_back(itr->path().generic_string());
        }
    }
    return true;
}

int main(int argc, char** argv) {
    try {
        bool update = false;
        std::string compiler_exec, tests_path;
        std::vector<std::string> subdir_list;
        for (int i = 1; i < argc; ++i) {
            std::string_view arg(argv[i]);
            if (arg == "--help") {
                // clang-format off
                static std::string_view text[] = {
                    "Usage: run_tests [options] <compiler-exec-path> <tests-path> <subdir-list>...",
                    "Options:",
                    "    --update       Rewrite test results.",
                    "    --help         Display this information.",
                };
                // clang-format on
                for (const auto& l : text) { std::cout << l << std::endl; }
                return 0;
            } else if (arg[0] != '-') {
                if (compiler_exec.empty()) {
                    compiler_exec = arg;
                } else if (tests_path.empty()) {
                    tests_path = arg;
                } else {
                    subdir_list.emplace_back(arg);
                }
            } else if (arg == "--update") {
                update = true;
            } else {
                std::cerr << "unknown flag `" << arg << "`" << std::endl;
                return -1;
            }
        }

        std::vector<std::string> test_lst;
        int ret_result = 0;
        auto old_current_path = std::filesystem::current_path();
        auto compiler_path = old_current_path / compiler_exec;
        std::filesystem::current_path(tests_path);
        if (!getTestList(".", subdir_list, test_lst)) { return -1; }

        unsigned busy_cnt = 0;
        size_t total_test_count = test_lst.size(), passed_test_count = 0;
        for (const auto& path : test_lst) {
            std::string opts;
            if (std::ifstream fopts((std::filesystem::path(path).remove_filename() / "opts.conf").string()); fopts) {
                std::getline(fopts, opts);
            }

            constexpr std::string_view busy = "|/-\\";
            std::cout << busy[busy_cnt] << '\b' << std::flush;
            busy_cnt = (busy_cnt + 1) & 3;

            const int result = std::system(
                (compiler_path.string() + " " + opts + " " + path + " 2>" + path + ".out").c_str());
            const bool must_fail = path.find("fail") != std::string::npos;
            const bool has_warn = path.find("warn") != std::string::npos;
            try {
                std::ifstream if1(path + ".out");
                if (!if1) { throw std::runtime_error("cannot open file `" + path + ".out" + "` for reading"); }
                std::string sout;
                std::copy(std::istreambuf_iterator<char>{if1}, std::istreambuf_iterator<char>{},
                          std::back_inserter(sout));
                if1.close();
                unsigned errs = 0, warns = 0;
                if (auto status_pos = sout.rfind("errors"); status_pos != std::string::npos) {
                    std::istringstream(sout.substr(status_pos + 6)) >> errs;
                } else {
                    throw std::runtime_error("invalid compiler output");
                }
                if (auto status_pos = sout.rfind("warnings"); status_pos != std::string::npos) {
                    std::istringstream(sout.substr(status_pos + 8)) >> warns;
                } else {
                    throw std::runtime_error("invalid compiler output");
                }
                if ((!must_fail && (result != 0 || errs != 0)) || (must_fail && (result == 0 || errs == 0))) {
                    throw std::runtime_error("inconsistent compilation result");
                } else if (!must_fail && ((!has_warn && warns != 0) || (has_warn && warns == 0))) {
                    throw std::runtime_error("compilation warnings");
                }
                if (!update) {
                    std::ifstream if2(path + ".out.expected");
                    if (!if2) {
                        throw std::runtime_error("cannot open file `" + path + ".out.expected" + "` for reading");
                    }
                    if (!std::equal(sout.begin(), sout.end(), std::istreambuf_iterator<char>{if2},
                                    std::istreambuf_iterator<char>{})) {
                        throw std::runtime_error("diffs with expected compiler output");
                    }
                } else {
                    std::ofstream of2(path + ".out.expected");
                    if (!of2) {
                        throw std::runtime_error("cannot open file `" + path + ".out.expected" + "` for writing");
                    }
                    std::copy(sout.begin(), sout.end(), std::ostreambuf_iterator<char>{of2});
                    std::cout << path << ": UPDATED!" << std::endl;
                }
                std::filesystem::remove(path + ".out");
                ++passed_test_count;
            } catch (const std::exception& ex) {
                std::cout << path << ": \033[0;31m" << ex.what() << "\033[0m" << std::endl;
                ret_result = -1;
            }
        }
        std::filesystem::current_path(old_current_path);
        if (passed_test_count == total_test_count) {
            std::cout << "Test result: "
                      << "\033[1;32m" << passed_test_count << '/' << total_test_count << " PASSED\033[0m" << std::endl;
        } else {
            std::cout << "Test result: "
                      << "\033[1;31m" << total_test_count - passed_test_count << '/' << total_test_count
                      << " FAILED\033[0m" << std::endl;
        }
        return ret_result;
    } catch (const std::exception& ex) { std::cout << ex.what() << std::endl; }
    return -1;
}
