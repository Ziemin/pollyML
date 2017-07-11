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
#include "llvm/IR/Constants.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"


#define DEBUG_TYPE "ProfilingInitializer"

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


llvm::GlobalVariable* createGlobalProfilingContextValue(llvm::Module& M) {

  IRBuilder<> builder{M.getContext()};

  ArrayRef<Type *> initProfilingArgs{};
  // int8* will model the void* from the profiling api
  llvm::PointerType* contextType = builder.getInt8Ty()->getPointerTo();

  // global variable to store the profiling context
  GlobalVariable *profilingContextPtr = new GlobalVariable(
      M,                                           // module
      contextType,                                 // value type
      false,                                       // isConstatn
      GlobalValue::PrivateLinkage,                 // linkage
      llvm::ConstantPointerNull::get(contextType), // initializer
      "__profiling_ctx");                          // name

  // create declaration of init_profiling function
  FunctionType *initProfilingType = FunctionType::get(
      contextType, initProfilingArgs, false);
  M.getOrInsertFunction("init_profiling", initProfilingType);
  Function *initProfilingFun = M.getFunction("init_profiling");

  // Define a function calling init_profiling and setting its result to
  // our global variable.
  FunctionType *prepareContextType = FunctionType::get(
      builder.getVoidTy(), initProfilingArgs, false);
  Function *prepareContextFun = Function::Create(
      prepareContextType,
      Function::PrivateLinkage,
      "__prepare_scop_profiling_context",
      &M);

  BasicBlock *prepareBody = BasicBlock::Create(
      M.getContext(), "body", prepareContextFun);
  builder.SetInsertPoint(prepareBody);
  CallInst* voidPtrCall = builder.CreateCall(initProfilingFun, {}, "CtxPtr");
  builder.CreateStore(voidPtrCall, profilingContextPtr, false);
  builder.CreateRetVoid();

  // Add this function to global llvm constructors
  appendToGlobalCtors(M, prepareContextFun, 0);

  return profilingContextPtr;
}

void createFinishProfilingCall(llvm::Module& M, llvm::GlobalVariable* ProfilingContext) {
  IRBuilder<> builder{M.getContext()};

  ArrayRef<Type *> finishProfilingArgs{ProfilingContext->getValueType()};

  // create declaration of finish_profiling function
  FunctionType *finishProfilingType = FunctionType::get(
      builder.getVoidTy(), finishProfilingArgs, false);
  M.getOrInsertFunction("finish_profiling", finishProfilingType);
  Function *finishProfilingFun= M.getFunction("finish_profiling");

  // Define a function calling finish_profiling
  FunctionType *callFinishType= FunctionType::get(
      builder.getVoidTy(), {}, false);
  Function *callFinishFun= Function::Create(
      callFinishType,
      Function::PrivateLinkage,
      "__finish_scop_profiling",
      &M);

  BasicBlock *finishBody = BasicBlock::Create(
      M.getContext(), "body", callFinishFun);
  builder.SetInsertPoint(finishBody);
  llvm::Value* loadedCtx = builder.CreateLoad(ProfilingContext, "CtxPtr");
  builder.CreateCall(finishProfilingFun, {loadedCtx});
  builder.CreateRetVoid();

  // Add this function to global llvm destructors
  appendToGlobalDtors(M, callFinishFun, 0);
}

} // namespace codegen
} // namespace pollyML
