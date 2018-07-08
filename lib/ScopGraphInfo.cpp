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

#include "pollyML/ScopGraphInfo.h"
#include "polly/DependenceInfo.h"
#include "polly/ScheduleOptimizer.h"
#include "polly/ScopInfo.h"
#include "polly/Support/SCEVValidator.h"
#include "polly/Support/ScopHelper.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instruction.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/FormatVariadic.h"
#include "llvm/Support/raw_ostream.h"
#include <string>

using namespace llvm;
using namespace polly;
using namespace pollyML;

#define DEBUG_TYPE "ScopGraphInfo"

char ScopGraphInfo::ID = 0;

bool ScopGraphInfo::runOnScop(Scop &S) { return false; }

void ScopGraphInfo::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequiredTransitive<ScopInfoRegionPass>();
  AU.addRequiredTransitive<DependenceInfo>();
  AU.setPreservesAll();
}

void ScopGraphInfo::printScop(raw_ostream &OS, Scop &S) const {
  OS << "Printing Scop Graph Information: " << S.getNameStr()
     << " for function: " << S.getFunction().getName() << '\n';
  // Params
  OS << "SCop Parameters: \n";
  for (const llvm::SCEV *Param : S.parameters()) {
    if (const SCEVUnknown *ValueParameter = dyn_cast<SCEVUnknown>(Param)) {
      Value *Val = ValueParameter->getValue();
      Val->getType()->print(OS);
    }

    OS << " - Name: " << *Param << " Type: ";
    Param->getType()->print(OS, true);
    OS << '\n';
  }
}

Pass *pollyML::createScopGraphInfoPass() { return new ScopGraphInfo(); }

INITIALIZE_PASS_BEGIN(ScopGraphInfo, "pollyML-scop-graph",
                      "PollyML - Analyze SCoP as a graph", false, true)
INITIALIZE_PASS_DEPENDENCY(ScopInfoRegionPass);
INITIALIZE_PASS_DEPENDENCY(DependenceInfo);
INITIALIZE_PASS_END(ScopGraphInfo, "pollyML-scop-graph",
                    "PollyML - Analyze SCoP as a graph", false, true)
