//===--- UseGnewCheck.h - clang-tidy-----------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_QEMU_USE_GNEW_H
#define LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_QEMU_USE_GNEW_H

#include "../ClangTidy.h"

namespace clang {
namespace tidy {
namespace qemu {

/// FIXME: Write a short description.
///
/// For the user-facing documentation see:
/// http://clang.llvm.org/extra/clang-tidy/checks/qemu-use-gnew.html
class UseGnewCheck : public ClangTidyCheck {
  const bool OnlyTypeExpr;
public:
  UseGnewCheck(StringRef Name, ClangTidyContext *Context)
      : ClangTidyCheck(Name, Context),
        OnlyTypeExpr(Options.get("OnlyTypeExpr", 0) != 0) {}
  void registerMatchers(ast_matchers::MatchFinder *Finder) override;
  void check(const ast_matchers::MatchFinder::MatchResult &Result) override;
};

} // namespace qemu
} // namespace tidy
} // namespace clang

#endif // LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_QEMU_USE_GNEW_H
