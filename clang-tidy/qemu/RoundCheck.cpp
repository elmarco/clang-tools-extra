//===--- RoundCheck.cpp - clang-tidy---------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "RoundCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Lex/Lexer.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace qemu {

AST_MATCHER(BinaryOperator, binaryOperatorIsInMacro) {
  return Node.getOperatorLoc().isMacroID();
}

void RoundCheck::registerMatchers(MatchFinder *Finder) {
  // (A + B-1) / B * B
  auto Add = ignoringParenImpCasts(binaryOperator(hasOperatorName("+"), unless(binaryOperatorIsInMacro())).bind("add"));
  auto Div = ignoringParenImpCasts(binaryOperator(hasOperatorName("/"), hasLHS(Add), unless(binaryOperatorIsInMacro())).bind("div"));
  Finder->addMatcher(binaryOperator(hasOperatorName("*"), hasLHS(Div)).bind("mul-align-up"), this);
  if (OnlyAlignUp)
      return;
  // (A + B-1) / B
  Div = ignoringParenImpCasts(binaryOperator(hasOperatorName("/"), hasLHS(Add)).bind("div-round-up"));
  Finder->addMatcher(expr(Div), this);
  // A / B * B
  Div = ignoringParenImpCasts(binaryOperator(hasOperatorName("/"), unless(binaryOperatorIsInMacro())).bind("div"));
  Finder->addMatcher(binaryOperator(hasOperatorName("*"), hasLHS(Div)).bind("mul-align-down"), this);
}

void RoundCheck::check(const MatchFinder::MatchResult &Result) {
  SourceManager &SM = *Result.SourceManager;
  LangOptions LangOpts = getLangOpts();

  if (Result.Nodes.getNodeAs<BinaryOperator>("mul-align-up")) {
    const auto *Mul = Result.Nodes.getNodeAs<BinaryOperator>("mul-align-up");
    const auto *Add = Result.Nodes.getNodeAs<BinaryOperator>("add");
    const auto *Div = Result.Nodes.getNodeAs<BinaryOperator>("div");
    llvm::APSInt DivValue, MulValue, AddValue;

    // FIXME only (A + B-1) / B * B form for now, could do symmetrics
    // FIXME with variable values
    if (!Add->getRHS()->EvaluateAsInt(AddValue, *Result.Context) ||
        !Div->getRHS()->EvaluateAsInt(DivValue, *Result.Context) ||
        !Mul->getRHS()->EvaluateAsInt(MulValue, *Result.Context))
      return;

    if (MulValue.getExtValue() != DivValue.getExtValue() ||
      MulValue.getExtValue() != (AddValue.getExtValue() + 1))
      return;

    llvm::SmallString<256> rep("QEMU_ALIGN_UP");
    if (MulValue.isPowerOf2())
      rep = "ROUND_UP";

    auto Diag = diag(Mul->getExprLoc(), "use %0 instead") << rep;

    rep += "(";
    rep += Lexer::getSourceText(CharSourceRange::getTokenRange(Add->getLHS()->getSourceRange()), SM, LangOpts);
    rep += ", ";
    rep += Lexer::getSourceText(CharSourceRange::getTokenRange(Mul->getRHS()->getSourceRange()), SM, LangOpts);
    rep += ")";

    Diag << FixItHint::CreateReplacement(Mul->getSourceRange(), rep);
  } else if (Result.Nodes.getNodeAs<BinaryOperator>("div-round-up")) {
      const auto *Div = Result.Nodes.getNodeAs<BinaryOperator>("div-round-up");
      const auto *Add = Result.Nodes.getNodeAs<BinaryOperator>("add");
      llvm::APSInt DivValue, AddValue;

      // FIXME only (A + B-1) / B
      if (!Add->getRHS()->EvaluateAsInt(AddValue, *Result.Context) ||
          !Div->getRHS()->EvaluateAsInt(DivValue, *Result.Context))
        return;

      if (DivValue.getExtValue() != (AddValue.getExtValue() + 1))
        return;

      llvm::SmallString<256> rep("DIV_ROUND_UP");
      auto Diag = diag(Div->getExprLoc(), "use %0 instead") << rep;
      rep += "(";
      rep += Lexer::getSourceText(CharSourceRange::getTokenRange(Add->getLHS()->getSourceRange()), SM, LangOpts);
      rep += ", ";
      rep += Lexer::getSourceText(CharSourceRange::getTokenRange(Div->getRHS()->getSourceRange()), SM, LangOpts);
      rep += ")";

      Diag << FixItHint::CreateReplacement(Div->getSourceRange(), rep);
  } else if (Result.Nodes.getNodeAs<BinaryOperator>("mul-align-down")) {
    const auto *Mul = Result.Nodes.getNodeAs<BinaryOperator>("mul-align-down");
    const auto *Div = Result.Nodes.getNodeAs<BinaryOperator>("div");

    // A / B * B
    auto TextDiv = Lexer::getSourceText(CharSourceRange::getTokenRange(Div->getRHS()->getSourceRange()), SM, LangOpts);
    auto TextMul = Lexer::getSourceText(CharSourceRange::getTokenRange(Mul->getRHS()->getSourceRange()), SM, LangOpts);

    if (TextDiv != TextMul) {
      llvm::APSInt DivValue, MulValue;
      if (!Div->getRHS()->EvaluateAsInt(DivValue, *Result.Context) ||
          !Mul->getRHS()->EvaluateAsInt(MulValue, *Result.Context))
        return;

      if (MulValue.getExtValue() != DivValue.getExtValue())
        return;
    }

    llvm::SmallString<256> rep("QEMU_ALIGN_DOWN");
    auto Diag = diag(Mul->getExprLoc(), "use %0 instead") << rep;
    rep += "(";
    rep += Lexer::getSourceText(CharSourceRange::getTokenRange(Div->getLHS()->getSourceRange()), SM, LangOpts);
    rep += ", ";
    rep += Lexer::getSourceText(CharSourceRange::getTokenRange(Mul->getRHS()->getSourceRange()), SM, LangOpts);
    rep += ")";

    Diag << FixItHint::CreateReplacement(Mul->getSourceRange(), rep);
  }
}

} // namespace qemu
} // namespace tidy
} // namespace clang
