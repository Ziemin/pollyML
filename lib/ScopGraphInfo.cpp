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
#include "polly/Support/GICHelper.h"
#include "polly/Support/SCEVValidator.h"
#include "polly/Support/ScopHelper.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instruction.h"
#include "llvm/Support/FormatVariadic.h"

#include <string>

using namespace llvm;
using namespace polly;
using namespace pollyML;

#define DEBUG_TYPE "ScopGraphInfo"

char ScopGraphInfo::ID = 0;

bool ScopGraphInfo::runOnScop(Scop &S) {
  D = &getAnalysis<DependenceInfo>().getDependences(Dependences::AL_Statement);
  return false;
}

void ScopGraphInfo::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequiredTransitive<ScopInfoRegionPass>();
  AU.addRequiredTransitive<DependenceInfo>();
  AU.setPreservesAll();
}

void ScopGraphInfo::printScop(raw_ostream &OS, Scop &S) const {
  OS << "===== Printing Scop Graph Information: " << S.getNameStr()
     << " for function: " << S.getFunction().getName() << '\n';

  // Params
  OS << formatv("- SCoP Context\n{0}\n", S.getContextStr());
  OS << formatv("- SCoP Assumed Context\n{0}\n", S.getAssumedContextStr());
  OS << formatv("- SCoP Invalid Context\n{0}\n", S.getInvalidContextStr());

  OS << formatv("- SCoP Param Space:\n{0}\n", S.getParamSpace().to_str());
  OS << formatv("- SCoP Full Param Space:\n{0}\n",
                S.getFullParamSpace().to_str());
  OS << "- SCop Parameters: \n";
  for (const llvm::SCEV *Param : S.parameters()) {
    if (const SCEVUnknown *ValueParameter = dyn_cast<SCEVUnknown>(Param)) {
      Value *Val = ValueParameter->getValue();
      Val->getType()->print(OS);
    }

    OS << " - Name: " << *Param
       << " ISL id: " << S.getIdForParam(Param).get_name() << " Type: ";
    Param->getType()->print(OS, true);
    OS << '\n';
  }

  OS << formatv("- Max Loop Depth: {0} \n", S.getMaxLoopDepth());

  // Domain
  OS << formatv("- SCoP Domain:\n{0}\n", S.getDomains().to_str());
  // Schedules
  OS << formatv("- SCoP Schedule:\n{0}\n", S.getSchedule().to_str());
  OS << formatv("- SCoP Schedule Tree:\n{0}\n", S.getScheduleTree().to_str());

  // Arrays
  OS << "- Arrays: \n";
  for (const auto *ArrInfo : S.arrays()) {
    OS.indent(4) << formatv("Array name: {0}\n",
                            ArrInfo->getBasePtrId().get_name());
    ArrInfo->print(OS.indent(4));
  }

  // Statements
  OS << formatv("- Scop Statements count: {0}\n", S.getSize());
  for (const auto &Stmt : S) {
    OS.indent(2) << formatv("Statement: {0}\n", Stmt.getDomainId().get_name());
    for (const auto *MemAcc : Stmt) {
      if (MemAcc->isRead())
        OS.indent(4) << "Access Type: READ\n";
      else
        OS.indent(4) << "Access Type: WRITE\n";

      OS.indent(4) << formatv("Access is reduction like: {0}\n",
                              MemAcc->isReductionLike());
      OS.indent(4) << formatv("Accessed array: {0}\n",
                              MemAcc->getArrayId().get_name());

      OS.indent(6) << formatv("Access relation: {0}\n",
                              MemAcc->getAccessRelation().to_str());
      OS.indent(6) << formatv("Address function: {0}\n",
                              MemAcc->getAddressFunction().to_str());
      OS.indent(6) << formatv(
          "Schedule on the relation: {0}\n",
          MemAcc->applyScheduleToAccessRelation(S.getSchedule()).to_str());
      OS.indent(6) << formatv("Access stride: {0}\n",
                              MemAcc->getStride(Stmt.getSchedule()).to_str());
    }
  }

  // Dependences
  OS << "- Dependences\n";
  D->print(OS);
}

Pass *pollyML::createScopGraphInfoPass() { return new ScopGraphInfo(); }

INITIALIZE_PASS_BEGIN(ScopGraphInfo, "pollyML-scop-graph",
                      "PollyML - Analyze SCoP as a graph", false, true)
INITIALIZE_PASS_DEPENDENCY(ScopInfoRegionPass);
INITIALIZE_PASS_DEPENDENCY(DependenceInfo);
INITIALIZE_PASS_END(ScopGraphInfo, "pollyML-scop-graph",
                    "PollyML - Analyze SCoP as a graph", false, true)
