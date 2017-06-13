//===--- UseGnewCheck.cpp - clang-tidy-------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#define DEBUG_TYPE "qemu-gnew"

#include "UseGnewCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Expr.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Lex/Lexer.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace qemu {

void UseGnewCheck::registerMatchers(MatchFinder *Finder) {
  auto Sizeof = expr(sizeOfExpr(anything())).bind("sizeof");
  auto Func = callee(functionDecl(
      anyOf(hasName("malloc"), hasName("calloc"), hasName("realloc"),
            hasName("alloca"), hasName("g_alloca"),
            hasName("g_malloc"), hasName("g_malloc0"), hasName("g_realloc"))));
  auto Alloc = callExpr(Func, hasArgument(0, Sizeof)).bind("alloc");
  auto AllocN = binaryOperator(hasParent(callExpr(Func).bind("alloc")),
                               hasOperatorName("*"),
                               hasEitherOperand(Sizeof)).bind("binop");

  Finder->addMatcher(Alloc, this);
  Finder->addMatcher(AllocN, this);

  Finder->addMatcher(cStyleCastExpr(
      anyOf(hasDescendant(Alloc), hasDescendant(AllocN))).bind("cast"), this);
}

void UseGnewCheck::check(const MatchFinder::MatchResult &Result) {
  SourceManager &SM = *Result.SourceManager;
  LangOptions LangOpts = getLangOpts();

  const auto *AllocExpr = Result.Nodes.getNodeAs<CallExpr>("alloc");
  const auto AllocName = AllocExpr->getCalleeDecl()->getAsFunction()->getName();
  const auto *SizeofExpr = Result.Nodes.getNodeAs<UnaryExprOrTypeTraitExpr>("sizeof");
  const auto *BinOp = Result.Nodes.getNodeAs<BinaryOperator>("binop");
  const auto *Cast = Result.Nodes.getNodeAs<CStyleCastExpr>("cast");

  llvm::SmallString<256> rep;
  if (AllocName == "calloc" || AllocName == "g_malloc0")
      rep = "g_new0";
  else if (AllocName == "alloca" || AllocName == "g_alloca")
      rep = "g_newa";
  else if (AllocName == "realloc" || AllocName == "g_realloc")
      rep = "g_renew";
  else
      rep = "g_new";

  auto Diag = diag(AllocExpr->getExprLoc(), "use %0() instead") << rep;

  rep += "(";
  if (SizeofExpr->isArgumentType()) {
    rep += SizeofExpr->getArgumentType().getAsString();
  } else if (OnlyTypeExpr) {
      return;
  } else {
    rep += "typeof(";
    rep += Lexer::getSourceText(
        CharSourceRange::getTokenRange(SizeofExpr->getArgumentExpr()->IgnoreParens()->getSourceRange()), SM, LangOpts);
    rep += ")";
  }
  rep += ", ";

  if (AllocName == "realloc" || AllocName == "g_realloc") {
    rep += Lexer::getSourceText(
        CharSourceRange::getTokenRange(AllocExpr->getArg(0)->getSourceRange()), SM, LangOpts);
    rep += ", ";
  }

  if (BinOp) {
      auto *OtherExpr = BinOp->getLHS() == SizeofExpr ? BinOp->getRHS() : BinOp->getLHS();
      StringRef OtherText = Lexer::getSourceText(
          CharSourceRange::getTokenRange(OtherExpr->getSourceRange()), SM, LangOpts);
      rep += OtherText;
  } else {
      rep += "1";
  }
  rep += ")";

  Diag << FixItHint::CreateReplacement(AllocExpr->getSourceRange(), rep);
  if (Cast)
      Diag << FixItHint::CreateReplacement(
          SourceRange(Cast->getLParenLoc(), Cast->getRParenLoc()), "");
}

} // namespace qemu
} // namespace tidy
} // namespace clang
