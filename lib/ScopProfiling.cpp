//===------- pollyML/ScopProfiling.cpp ------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Inject profiling code into Scops
//
//===----------------------------------------------------------------------===//

#include "pollyML/ScopProfiling.h"
#include "pollyML/ProfilingCodegen.h"
#include "polly/DependenceInfo.h"
#include "polly/ScopInfo.h"
#include "polly/Support/ScopHelper.h"
#include "polly/Support/SCEVValidator.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instruction.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/Format.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include <string>

using namespace llvm;
using namespace polly;
using namespace pollyML;

#define DEBUG_TYPE "ScopProfiling"

char ScopProfiling::ID = 0;

bool ScopProfiling::runOnScop(Scop &S) {
  DEBUG(errs() << "Injecting profiling code inside function: "
    << S.getFunction().getName() << " for SCoP: " << S.getName()
    << '\n');

  // may return null, if so use EntryBlock
  BasicBlock *ScopEntry = S.getEnteringBlock();
  if (ScopEntry == nullptr) {
    ScopEntry = S.getEntry();
  }
  BasicBlock *ScopExit = S.getExit();

  IRBuilder<> Builder{ScopEntry->getContext()};
  Builder.SetInsertPoint(ScopEntry, --ScopEntry->end());
  // TODO Insert startScop call
  Builder.SetInsertPoint(ScopExit, ScopExit->getFirstInsertionPt());
  // TODO Insert stopScop call
  return true;
}

void ScopProfiling::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequiredTransitive<ScopInfoRegionPass>();
  AU.setPreservesAll();
}

void ScopProfiling::printScop(raw_ostream &OS, Scop &S) const {
  OS << "Printing Scop: " << S.getNameStr()
     << " for function: " << S.getFunction().getName() << '\n';
  BasicBlock *ScopEntry = S.getEntry();
  BasicBlock *ScopExit = S.getExit();
  OS << "-- Scop entry: " << ScopEntry->getName() << '\n';
  OS << "-- Scop exit: " << ScopExit->getName() << '\n';
  OS << "SCop Parameters: \n";
  for (const llvm::SCEV* Param: S.parameters()) {
    if (const SCEVUnknown *ValueParameter = dyn_cast<SCEVUnknown>(Param)) {
      Value *Val = ValueParameter->getValue();
      Val->getType()->print(OS);
    }

    OS << " - Name: " << *Param << " Type: ";
    Param->getType()->print(OS, true);
    OS << '\n';
  }
}

Pass *pollyML::createScopProfilingPass() {
  return new ScopProfiling();
}

INITIALIZE_PASS_BEGIN(ScopProfiling, "pollyML-scop-profile",
                      "PollyML - Inject Profiling Code to detected Scops",
                      false, false)
INITIALIZE_PASS_DEPENDENCY(ScopInfoRegionPass);
INITIALIZE_PASS_END(ScopProfiling, "pollyML-scop-profile",
                    "PollyML - Inject Profiling Code to detected Scops", false,
                    false)
