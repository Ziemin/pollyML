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
#include "pollyML/ProfilingInitializer.h"
#include "polly/DependenceInfo.h"
#include "polly/ScopInfo.h"
#include "polly/Support/ScopHelper.h"
#include "polly/Support/SCEVValidator.h"
#include "polly/ScheduleOptimizer.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instruction.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/FormatVariadic.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include <string>

using namespace llvm;
using namespace polly;
using namespace pollyML;

#define DEBUG_TYPE "ScopProfiling"


char ScopProfiling::ID = 0;

bool ScopProfiling::runOnScop(Scop &S) {
  // name identified as concatenation of a function name and a scop name
  std::string regionName =
      formatv("{0}___{1}", S.getFunction().getName(), S.getName());

  DEBUG(errs() << "Injecting profiling code inside function: "
               << S.getFunction().getName() << " for SCoP: " << S.getName()
               << '\n');

  // -- Call start_scop profiling function
  // may return null, if so, use EntryBlock
  BasicBlock *ScopEntry = S.getEnteringBlock();
  if (ScopEntry == nullptr) {
    ScopEntry = S.getEntry();
  }
  SmallVector<std::string, 3> ParameterNames;
  SmallVector<Value *, 3> ParameterValues;

  for (const SCEV *Param : S.parameters()) {
    std::string paramName;
    raw_string_ostream OS(paramName);
    Param->print(OS);
    OS.flush();
    if (const SCEVUnknown *ValueParameter = dyn_cast<SCEVUnknown>(Param)) {
      DEBUG(errs() << "Adding Scop parameter: " << paramName << '\n');
      Value *Val = ValueParameter->getValue();
      ParameterValues.push_back(Val);
      ParameterNames.push_back(std::move(paramName));
    } else {
      DEBUG(errs() << "Parameter " << paramName << " is not SCEVUnknown. "
                   << "Not adding it to the list!");
    }
  }
  Module &M = *ScopEntry->getModule();
  Value *RegionNamePtr = codegen::createStartProfilingCall(
      M, *ScopEntry, regionName, ParameterNames, ParameterValues, scopCount);

  // -- Call stop_scop profiling function
  BasicBlock *ScopExit = S.getExit();
  codegen::createStopProfilingCall(M, *ScopExit, RegionNamePtr);

  scopCount++;

  return true;
}

void ScopProfiling::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequiredTransitive<ScopInfoRegionPass>();
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
INITIALIZE_PASS_DEPENDENCY(ProfilingInitializer);
INITIALIZE_PASS_END(ScopProfiling, "pollyML-scop-profile",
                    "PollyML - Inject Profiling Code to detected Scops", false,
                    false)
