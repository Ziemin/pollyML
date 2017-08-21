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
#include "pollyML/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Constants.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FormatVariadic.h"
#include <array>


#define DEBUG_TYPE "ProfilingCodegen"

using namespace llvm;
using namespace pollyML;

namespace pollyML {
namespace codegen {


void createStartAndStopProfilingDeclarations(llvm::Module& M) {
  IRBuilder<> builder{M.getContext()};

  /// create start_scop declaration
  std::array<Type *, 5> startScopArgumentsTy{{
    builder.getInt8Ty()->getPointerTo(),                 // context
    builder.getInt8Ty()->getPointerTo(),                 // region_name
    builder.getInt32Ty(),                                // param_count
    builder.getInt8Ty()->getPointerTo()->getPointerTo(), // parameter_names
    builder.getInt64Ty()->getPointerTo()}};              // parameter_values
  FunctionType *startScopFunType = FunctionType::get(
      builder.getVoidTy(), startScopArgumentsTy, false);
  M.getOrInsertFunction(START_SCOP_FUN_NAME, startScopFunType);

  /// create stop_scop declaration
  std::array<Type *, 2> stopScopArgumentsTy{{
    builder.getInt8Ty()->getPointerTo(),                 // context
    builder.getInt8Ty()->getPointerTo()}};               // region_name
  FunctionType *stopScopFunType = FunctionType::get(
      builder.getVoidTy(), stopScopArgumentsTy, false);
  M.getOrInsertFunction(STOP_SCOP_FUN_NAME, stopScopFunType);
}


llvm::GlobalVariable* createGlobalProfilingContextValue(
    llvm::Module& M, llvm::StringRef ConfigFile) {

  IRBuilder<> builder{M.getContext()};

  // -- context global variable preparation
  // int8* will model the void* from the profiling api
  llvm::PointerType* contextType = builder.getInt8Ty()->getPointerTo();

  // global variable to store the profiling context
  GlobalVariable *profilingContextPtr = new GlobalVariable(
      M,                                           // module
      contextType,                                 // value type
      false,                                       // isConstant
      GlobalValue::LinkOnceAnyLinkage,             // linkage
      llvm::ConstantPointerNull::get(contextType), // initializer
      PROFILING_CONTEXT_VAR_NAME);                 // name

  // -- create declaration of init_profiling function
  std::array<Type *, 1> initProfilingArgs{{
    builder.getInt8Ty()->getPointerTo()  // path to configuration file
  }};

  FunctionType *initProfilingType = FunctionType::get(
      contextType, initProfilingArgs, false);
  M.getOrInsertFunction(INIT_PROFILING_FUN_NAME, initProfilingType);
  Function *initProfilingFun = M.getFunction(INIT_PROFILING_FUN_NAME);

  // -- Define a function calling init_profiling and setting its result to
  // -- our global variable.
  FunctionType *prepareContextType = FunctionType::get(
      builder.getVoidTy(), {}, false);
  Function *prepareContextFun = Function::Create(
      prepareContextType,
      Function::LinkOnceAnyLinkage,
      "__prepare_scop_profiling_context",
      &M);

  BasicBlock *prepareBody = BasicBlock::Create(
      M.getContext(), "body", prepareContextFun);
  builder.SetInsertPoint(prepareBody);

  // -- arguments preparation
  Value* configFilePath = builder.CreateGlobalStringPtr(
      ConfigFile, "__scop_config_file");


  std::array<Value*, 1> callArgs{{configFilePath}};

  // -- actual call to init_profiling
  CallInst* voidPtrCall = builder.CreateCall(
      initProfilingFun,
      callArgs,
      "CtxPtr");
  builder.CreateStore(voidPtrCall, profilingContextPtr, false);
  builder.CreateRetVoid();

  // Add this function to global llvm constructors
  appendToGlobalCtors(M, prepareContextFun, 0);

  return profilingContextPtr;
}


void createFinishProfilingCall(Module& M,
                               GlobalVariable* ProfilingContext,
                               llvm::StringRef OutputFilePath) {

  IRBuilder<> builder{M.getContext()};

  std::array<Type *, 2> finishProfilingArgs{
    {ProfilingContext->getValueType(), builder.getInt8Ty()->getPointerTo()}
  };

  // create declaration of finish_profiling function
  FunctionType *finishProfilingType = FunctionType::get(
      builder.getVoidTy(), finishProfilingArgs, false);
  M.getOrInsertFunction(FINISH_PROFILING_FUN_NAME, finishProfilingType);
  Function *finishProfilingFun = M.getFunction(FINISH_PROFILING_FUN_NAME);

  // Define a function calling finish_profiling
  FunctionType *callFinishType = FunctionType::get(
      builder.getVoidTy(), {}, false);
  Function *callFinishFun = Function::Create(
      callFinishType,
      Function::LinkOnceAnyLinkage,
      "__finish_scop_profiling",
      &M);


  BasicBlock *finishBody = BasicBlock::Create(
      M.getContext(), "body", callFinishFun);
  builder.SetInsertPoint(finishBody);

  Value* outputFilePathVal = builder.CreateGlobalStringPtr(
      OutputFilePath, "__scop_output_file_path");

  llvm::Value* loadedCtx = builder.CreateLoad(ProfilingContext, "CtxPtr");
  builder.CreateCall(finishProfilingFun, {loadedCtx, outputFilePathVal});
  builder.CreateRetVoid();

  // Add this function to global llvm destructors
  appendToGlobalDtors(M, callFinishFun, 0);
}


Value* createStartProfilingCall(
    llvm::Module& M,
    llvm::BasicBlock& ScopEntry,
    llvm::StringRef RegionName,
    llvm::ArrayRef<std::string> ParameterNames,
    llvm::ArrayRef<Value*> ParameterValues,
    int ScopNumber) {

  GlobalVariable *ProfilingContext = M.getGlobalVariable(PROFILING_CONTEXT_VAR_NAME);
  assert(ProfilingContext && "Global variable with profiling context  should be available in the module");

  LLVMContext& context = M.getContext();
  IRBuilder<> builder{context};
  builder.SetInsertPoint(&ScopEntry, --ScopEntry.end());

  assert(ParameterNames.size() == ParameterValues.size() &&
         "The number of parameter names and their values has to be the same");

  // allocate arrays of strings
  ArrayType* namesArrTy = ArrayType::get(builder.getInt8Ty()->getPointerTo(),
                                         ParameterNames.size());
  SmallVector<Constant*, 3> initNames;
  uint ind = 0;
  for (auto &paramName: ParameterNames) {
    Value* paramStrVal = builder.CreateGlobalStringPtr(
        paramName, formatv("__scop_{0}_param_p_{1}", ScopNumber, ind));
    initNames.push_back(dyn_cast<Constant>(paramStrVal));
    ind++;
  }
  GlobalVariable* paramNamesVar = new GlobalVariable(
      M,
      namesArrTy,
      true,
      GlobalValue::PrivateLinkage,
      ConstantArray::get(namesArrTy, initNames),
      formatv("__scop_{0}_param_names", ScopNumber));

  // allocate array of int64 values
  ArrayType* valuesArrTy = ArrayType::get(builder.getInt64Ty(),
                                          ParameterValues.size());
  SmallVector<uint64_t, 3> initValues(ParameterValues.size(), 0);
  GlobalVariable* paramValuesVar = new GlobalVariable(
      M,
      valuesArrTy,
      false,
      GlobalValue::PrivateLinkage,
      ConstantDataArray::get(context, initValues),
      formatv("__scop_{0}_param_values", ScopNumber));

  // get function to call
  Function* start_scopFun = M.getFunction(START_SCOP_FUN_NAME);
  assert(start_scopFun && "start_scop function has to be declared in the module");

  // -- set parameter values
  for (uint valInd = 0; valInd < ParameterValues.size(); valInd++) {
    Value* paramAddr = builder.CreateConstGEP2_32(
        valuesArrTy, paramValuesVar, 0, valInd, "p_" + std::to_string(valInd));

    // SExt instruction to promote integers to i64
    Value* paramIni64 = builder.CreateSExt(ParameterValues[valInd], builder.getInt64Ty());
    builder.CreateStore(paramIni64, paramAddr);
  }

  // -- prepare arguments
  Value* loadedCtxArg = builder.CreateLoad(ProfilingContext, "CtxPtr");
  Value* regionNameArg = builder.CreateGlobalStringPtr(
      RegionName, formatv("__scop_{0}_region_name", ScopNumber));
  ConstantInt* paramCountArg = builder.getInt32(ParameterNames.size());
  paramCountArg->setName("param_count");
  Value* ptrToNamesArg = builder.CreateConstGEP2_32(namesArrTy, paramNamesVar, 0, 0);
  Value* ptrToValuesArg = builder.CreateConstGEP2_32(valuesArrTy, paramValuesVar, 0, 0);
  std::array<Value*, 5> callArgs{{loadedCtxArg,
                                  regionNameArg,
                                  paramCountArg,
                                  ptrToNamesArg,
                                  ptrToValuesArg}};

  // create actual call
  builder.CreateCall(start_scopFun, callArgs);

  return regionNameArg;
}


void createStopProfilingCall(
    llvm::Module& M,
    llvm::BasicBlock& ScopExit,
    llvm::Value* RegionNamePtr) {

  GlobalVariable *ProfilingContext = M.getGlobalVariable(PROFILING_CONTEXT_VAR_NAME);
  assert(ProfilingContext && "Global variable with profiling context  should be available in the module");

  LLVMContext& context = M.getContext();
  IRBuilder<> builder{context};
  builder.SetInsertPoint(&ScopExit, ScopExit.getFirstInsertionPt());

  // get function to call
  Function* stop_scopFun = M.getFunction(STOP_SCOP_FUN_NAME);
  assert(stop_scopFun && "stop_scop function has to be declared in the module");

  Value* loadedCtxArg = builder.CreateLoad(ProfilingContext, "CtxPtr");
  std::array<Value*, 2> callArgs{{loadedCtxArg, RegionNamePtr}};

  builder.CreateCall(stop_scopFun, callArgs);
}

} // namespace codegen
} // namespace pollyML
