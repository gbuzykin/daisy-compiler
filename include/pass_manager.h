#pragma once

#include <string_view>
#include <vector>

#define DAISY_ADD_PASS(pass_type) static daisy::pass_type g_pass_##pass_type

namespace daisy {

class Pass;
struct CompilationContext;

enum class PassResult { kSuccess = 0, kError, kFatalError };

class PassManager {
 public:
    static PassManager& getInstance();
    Pass* findPassByName(std::string_view name) const;
    void configure();
    PassResult run(CompilationContext& ctx);

 private:
    std::vector<Pass*> passes_;
};

class Pass {
 public:
    Pass() : next_avail_(first_avail_) { first_avail_ = this; }
    void enable() { is_enabled_ = true; }
    void disable() { is_enabled_ = false; }
    bool isEnabled() const { return is_enabled_; }
    virtual std::string_view getName() const = 0;
    virtual void configure() = 0;
    virtual PassResult run(CompilationContext& ctx) = 0;
    virtual void cleanup() = 0;
    virtual ~Pass() = default;

 private:
    friend class PassManager;
    static Pass* first_avail_;
    Pass* next_avail_;
    bool is_enabled_ = true;
};

}  // namespace daisy
