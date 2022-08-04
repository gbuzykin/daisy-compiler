#include "pass_manager.h"

#include "uxs/algorithm.h"

using namespace daisy;

/*static*/ Pass* Pass::first_avail_ = nullptr;

/*static*/ PassManager& PassManager::getInstance() {
    static PassManager pass_manager;
    return pass_manager;
}

Pass* PassManager::findPassByName(std::string_view name) const {
    auto [it, found] = uxs::find_if(passes_, [name](auto* pass) { return pass->getName() == name; });
    if (found) { return *it; }
    return nullptr;
}

void PassManager::configure() {
    passes_.reserve(32);
    for (auto* pass = Pass::first_avail_; pass; pass = pass->next_avail_) { passes_.push_back(pass); }
    for (auto* pass : passes_) { pass->configure(); }
}

PassResult PassManager::run(CompilationContext& ctx) {
    PassResult result = PassResult::kSuccess;
    for (auto* pass : passes_) {
        PassResult pass_result = pass->run(ctx);
        pass->cleanup();
        switch (pass_result) {
            case PassResult::kError: result = PassResult::kError; break;
            case PassResult::kFatalError: return PassResult::kFatalError;
            default: break;
        }
    }
    return result;
}
