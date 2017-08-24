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
  std::array<Type *, 4> startScopArgumentsTy{{
    builder.getInt8Ty()->getPointerTo(),                 // region_name
    builder.getInt32Ty(),                                // param_count
    builder.getInt8Ty()->getPointerTo()->getPointerTo(), // parameter_names
    builder.getInt64Ty()->getPointerTo()}};              // parameter_values
  FunctionType *startScopFunType = FunctionType::get(
      builder.getVoidTy(), startScopArgumentsTy, false);
  M.getOrInsertFunction(START_SCOP_FUN_NAME, startScopFunType);

  /// create stop_scop declaration
  std::array<Type *, 1> stopScopArgumentsTy{{
    builder.getInt8Ty()->getPointerTo()}};               // region_name
  FunctionType *stopScopFunType = FunctionType::get(
      builder.getVoidTy(), stopScopArgumentsTy, false);
  M.getOrInsertFunction(STOP_SCOP_FUN_NAME, stopScopFunType);
}


void createInitProfilingCall(llvm::Module& M) {

  IRBuilder<> builder{M.getContext()};

  FunctionType *initProfilingType = FunctionType::get(
      builder.getVoidTy(), {}, false);
  M.getOrInsertFunction(INIT_PROFILING_FUN_NAME, initProfilingType);
  Function *initProfilingFun = M.getFunction(INIT_PROFILING_FUN_NAME);

  // -- Define a function calling init_profiling and setting its result to
  // -- our global variable.
  FunctionType *callInitProfilingType = FunctionType::get(
      builder.getVoidTy(), {}, false);
  Function *callInitProfilingFun = Function::Create(
      callInitProfilingType,
      Function::LinkOnceAnyLinkage,
      "__init_scop_profiling",
      &M);

  BasicBlock *prepareBody = BasicBlock::Create(
      M.getContext(), "body", callInitProfilingFun);
  builder.SetInsertPoint(prepareBody);


  // -- actual call to init_profiling
  builder.CreateCall(initProfilingFun, {});
  builder.CreateRetVoid();

  // Add this function to global llvm constructors
  appendToGlobalCtors(M, callInitProfilingFun, 0);
}


void createFinishProfilingCall(Module& M) {

  IRBuilder<> builder{M.getContext()};

  // create declaration of finish_profiling function
  FunctionType *finishProfilingType = FunctionType::get(
      builder.getVoidTy(), {}, false);
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

  builder.CreateCall(finishProfilingFun, {});
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
  Value* regionNameArg = builder.CreateGlobalStringPtr(
      RegionName, formatv("__scop_{0}_region_name", ScopNumber));
  ConstantInt* paramCountArg = builder.getInt32(ParameterNames.size());
  paramCountArg->setName("param_count");
  Value* ptrToNamesArg = builder.CreateConstGEP2_32(namesArrTy, paramNamesVar, 0, 0);
  Value* ptrToValuesArg = builder.CreateConstGEP2_32(valuesArrTy, paramValuesVar, 0, 0);
  std::array<Value*, 4> callArgs{{regionNameArg,
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

  LLVMContext& context = M.getContext();
  IRBuilder<> builder{context};
  builder.SetInsertPoint(&ScopExit, ScopExit.getFirstInsertionPt());

  // get function to call
  Function* stop_scopFun = M.getFunction(STOP_SCOP_FUN_NAME);
  assert(stop_scopFun && "stop_scop function has to be declared in the module");

  std::array<Value*, 1> callArgs{{RegionNamePtr}};

  builder.CreateCall(stop_scopFun, callArgs);
}

} // namespace codegen
} // namespace pollyML
