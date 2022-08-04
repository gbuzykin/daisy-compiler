#include "ctx/ctx.h"
#include "pass_manager.h"
#include "uxs/format.h"

using namespace daisy;

int main(int argc, char** argv) {
    auto ctx = std::make_unique<CompilationContext>("");
    PassManager::getInstance().configure();
    PassManager::getInstance().run(*ctx);
    return 0;
}
