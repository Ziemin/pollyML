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
#include "llvm/Support/Format.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include <string>

using namespace llvm;
using namespace polly;
using namespace pollyML;

char ScopProfiling::ID = 0;

bool ScopProfiling::runOnScop(Scop &S) {
  // may return null, if so use EntryBlock
  BasicBlock *ScopEntry = S.getEnteringBlock();
  if (ScopEntry == nullptr) {
    ScopEntry = S.getEntry();
  }
  BasicBlock *ScopExit = S.getExit();

  IRBuilder<> Builder{ScopEntry->getContext()};
  const std::string BeginMessage =
      "Before SCOP: " + S.getNameStr() +
      " in function: " + S.getFunction().getName().str() + '\n';
  const std::string EndMessage =
      "After SCOP: " + S.getNameStr() +
      " in function: " + S.getFunction().getName().str() + '\n';

  Builder.SetInsertPoint(ScopEntry, --ScopEntry->end());
  codegen::createPrintCall(Builder, BeginMessage);
  Builder.SetInsertPoint(ScopExit, ScopExit->getFirstInsertionPt());
  codegen::createPrintCall(Builder, EndMessage);
  return true;
}

void ScopProfiling::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequiredTransitive<ScopInfoRegionPass>();
  // AU.setPreservesAll();
}

void ScopProfiling::printScop(raw_ostream &OS, Scop &S) const {
  OS << "Printing Scop: " << S.getNameStr()
     << " for function: " << S.getFunction().getName() << '\n';
  BasicBlock *ScopEntry = S.getEntry();
  BasicBlock *ScopExit = S.getExit();
  OS << "-- Scop entry: " << ScopEntry->getName() << '\n';
  OS << "-- Scop exit: " << ScopExit->getName() << '\n';
  OS << "SCop Parameters: ";
  for (const llvm::SCEV* Param: S.parameters()) {
    std::string ParameterName;
    if (const SCEVUnknown *ValueParameter = dyn_cast<SCEVUnknown>(Param)) {
      Value *Val = ValueParameter->getValue();
      CallInst *Call = dyn_cast<CallInst>(Val);

      if (Call && isConstCall(Call)) {
        ParameterName = "Some call";
        //ParameterName = getCallParamName(Call);
      } else if (UseInstructionNames) {
        // If this parameter references a specific Value and this value has a name
        // we use this name as it is likely to be unique and more useful than just
        // a number.
        if (Val->hasName())
          ParameterName = Val->getName();
        else if (LoadInst *LI = dyn_cast<LoadInst>(Val)) {
          auto *LoadOrigin = LI->getPointerOperand()->stripInBoundsOffsets();
          if (LoadOrigin->hasName()) {
            ParameterName += "_loaded_from_";
            ParameterName +=
                LI->getPointerOperand()->stripInBoundsOffsets()->getName();
          }
        }
      }
    }

    OS << " - Name: " << ParameterName << " Type: \n";
    Param->getType()->print(OS);
  }
}

INITIALIZE_PASS_BEGIN(ScopProfiling, "pollyML-scop-profile",
                      "PollyML - Inject Profiling Code to detected Scops",
                      false, false)
INITIALIZE_PASS_DEPENDENCY(ScopInfoRegionPass);
INITIALIZE_PASS_END(ScopProfiling, "pollyML-scop-profile",
                    "PollyML - Inject Profiling Code to detected Scops", false,
                    false)
