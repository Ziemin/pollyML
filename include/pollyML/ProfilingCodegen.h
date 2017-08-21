//===--- ProfilingCodegen.h ----------------------------------- LLVM-IR ---===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// These functions are used to insert code to profiling functions into LLVM-IR
//
//===----------------------------------------------------------------------===//

#pragma once

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/IRBuilder.h"
#include <utility>

namespace llvm {
class Module;
class Value;
class BasicBlock;
} // namespace llvm

namespace pollyML {
namespace codegen {

/// Declares start_scop and stop_scop functions from the scop profiling library.
void createStartAndStopProfilingDeclarations(llvm::Module &M);

/// This creates a global value inside the module with the given name.
/// The value is initialized by calling the init_profiling function from the
/// profiling library.
/// This call is added to module constructors 'llvm.global_ctors'
llvm::GlobalVariable *createGlobalProfilingContextValue(
    llvm::Module &M, llvm::StringRef ConfigFile);

/// This creates a call to finish_profiling function from the profiling library.
/// This call is added to module destructors 'llvm.global_dtors'
void createFinishProfilingCall(llvm::Module &M,
                               llvm::GlobalVariable *ProfilingContext,
                               llvm::StringRef OutputFilePath);

/// This creates a call to start_scop function from the profiling library.
/// Two arrays with names and parameter values are allocated for every scop.
///
/// Return pointer to global string with region name
llvm::Value *createStartProfilingCall(
    llvm::Module &M, llvm::BasicBlock &ScopEntry, llvm::StringRef RegionName,
    llvm::ArrayRef<std::string> ParameterNames,
    llvm::ArrayRef<llvm::Value *> ParameterValues, int ScopNumber);

/// This creates a call to stop_scop function from the profiling library.
void createStopProfilingCall(llvm::Module &M, llvm::BasicBlock &ScopExit,
                             llvm::Value *RegionNamePtr);

} // namespace codegen
} // namespace pollyML
