//===--- ProfilingCodegen.cpp --------------------------------- LLVM-IR ---===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//===----------------------------------------------------------------------===//

#include "pollyML/ProfilingCodegen.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Module.h"

using namespace llvm;
using namespace pollyML;

namespace pollyML {
namespace codegen {

namespace {

Constant *getPutsFunction(IRBuilder<> &Builder) {
  Module *M = Builder.GetInsertBlock()->getParent()->getParent();
  ArrayRef<Type *> PutsArgs{Builder.getInt8Ty()->getPointerTo()};
  FunctionType *PutsType =
      FunctionType::get(Builder.getInt32Ty(), PutsArgs, false);

  return M->getOrInsertFunction("puts", PutsType);
}

} // namespace

void createPrintCall(IRBuilder<> &Builder, llvm::StringRef Text) {
  Value *TextToPrint = Builder.CreateGlobalStringPtr(Text);
  Constant *PutsFun = getPutsFunction(Builder);

  Builder.CreateCall(PutsFun, TextToPrint);
}

} // namespace codegen
} // namespace pollyML
