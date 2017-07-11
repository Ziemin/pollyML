//===------- pollyML/ScopProfiling.h - Scop Profiling Code ----- *- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Inject profiling code into SCoPs
//
//===----------------------------------------------------------------------===//

#pragma once

#include "polly/ScopPass.h"

namespace pollyML {

class ScopProfiling : public polly::ScopPass {

public:
  static char ID;

  ScopProfiling() : polly::ScopPass(ID) {}

  bool runOnScop(polly::Scop &S) override;

  void getAnalysisUsage(llvm::AnalysisUsage &AU) const override;

  void printScop(llvm::raw_ostream &OS, polly::Scop &S) const override;
};

llvm::Pass *createScopProfilingPass();

} // namespace pollyML

namespace llvm {
class PassRegistry;
void initializeScopProfilingPass(llvm::PassRegistry &);
} // namespace llvm
