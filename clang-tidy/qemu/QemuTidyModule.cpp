//===--- QemuTidyModule.cpp - clang-tidy ----------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "../ClangTidy.h"
#include "../ClangTidyModule.h"
#include "../ClangTidyModuleRegistry.h"
#include "RoundCheck.h"
#include "UseBoolLiteralsCheck.h"
#include "UseGnewCheck.h"

namespace clang {
namespace tidy {
namespace qemu {

class QemuModule : public ClangTidyModule {
public:
  void addCheckFactories(ClangTidyCheckFactories &CheckFactories) override {
    CheckFactories.registerCheck<RoundCheck>(
        "qemu-round");
    CheckFactories.registerCheck<UseBoolLiteralsCheck>(
        "qemu-use-bool-literals");
    CheckFactories.registerCheck<UseGnewCheck>(
        "qemu-use-gnew");
  }
};

// Register the QemuModule using this statically initialized variable.
static ClangTidyModuleRegistry::Add<QemuModule>
    X("qemu-module", "Adds qemu checks.");

} // namespace qemu

// This anchor is used to force the linker to link in the generated object file
// and thus register the QemuModule.
volatile int QemuModuleAnchorSource = 0;

} // namespace tidy
} // namespace clang
