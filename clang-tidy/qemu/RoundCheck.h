//===--- RoundCheck.h - clang-tidy-------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_QEMU_ROUND_H
#define LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_QEMU_ROUND_H

#include "../ClangTidy.h"

namespace clang {
namespace tidy {
namespace qemu {

/// FIXME: Write a short description.
///
/// For the user-facing documentation see:
/// http://clang.llvm.org/extra/clang-tidy/checks/qemu-round.html
class RoundCheck : public ClangTidyCheck {
  const bool OnlyAlignUp;
public:
  RoundCheck(StringRef Name, ClangTidyContext *Context)
      : ClangTidyCheck(Name, Context),
        OnlyAlignUp(Options.get("OnlyAlignUp", 0) != 0)
  {}
  void storeOptions(ClangTidyOptions::OptionMap &Opts) override {
    Options.store(Opts, "OnlyAlignUp", OnlyAlignUp);
  }
  void registerMatchers(ast_matchers::MatchFinder *Finder) override;
  void check(const ast_matchers::MatchFinder::MatchResult &Result) override;
};

} // namespace qemu
} // namespace tidy
} // namespace clang

#endif // LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_QEMU_ROUND_H
