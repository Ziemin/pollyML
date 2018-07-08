//===------- pollyML/ScopGraph.h - Scop as a Graph Analysis ---- *- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Analyze Scops in terms of statements graphs
//
//===----------------------------------------------------------------------===//

#pragma once

#include "polly/ScopPass.h"

namespace pollyML {

class ScopGraphInfo : public polly::ScopPass {

public:
  static char ID;

  ScopGraphInfo() : polly::ScopPass(ID) {}

  bool runOnScop(polly::Scop &S) override;

  void getAnalysisUsage(llvm::AnalysisUsage &AU) const override;

  void printScop(llvm::raw_ostream &OS, polly::Scop &S) const override;

  using polly::ScopPass::doFinalization;
  bool doFinalization(llvm::Module &) override {
    scopCount = 0;
    return true;
  }

private:
  int scopCount = 0;
};

llvm::Pass *createScopGraphInfoPass();

} // namespace pollyML

namespace llvm {
class PassRegistry;
void initializeScopGraphInfoPass(llvm::PassRegistry &);
} // namespace llvm
