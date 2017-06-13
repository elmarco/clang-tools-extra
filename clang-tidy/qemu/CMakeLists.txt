set(LLVM_LINK_COMPONENTS support)

add_clang_library(clangTidyQemuModule
  QemuTidyModule.cpp
  RoundCheck.cpp
  UseBoolLiteralsCheck.cpp
  UseGnewCheck.cpp

  LINK_LIBS
  clangAST
  clangASTMatchers
  clangBasic
  clangLex
  clangTidy
  clangTidyMiscModule
  clangTidyUtils
  clangTooling
  )
