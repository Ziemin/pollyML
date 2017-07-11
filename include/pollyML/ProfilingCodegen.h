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

#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/IRBuilder.h"
#include <utility>

namespace llvm {
  class Module;
  class Value;
}

namespace pollyML {
namespace codegen {

/// This creates a call to start_scop function from the profiling library.
//
/// Two arrays with names and parameter values are allocated for every scop.
void createStartProfilingCall(
    llvm::IRBuilder<> &Builder,
    llvm::StringRef RegionName,
    llvm::Value* ProfilingContext,
    llvm::SmallVector<llvm::StringRef, 3> ParameterNames,
    llvm::SmallVector<llvm::Value*, 3> ParameterValues);

/// This creates a call to stop_scop function from the profiling library.
void createStopProfilingCall(
    llvm::IRBuilder<> &Builder,
    llvm::StringRef RegionName,
    llvm::Value* ProfilingContext);


/// This creates a global value inside the module with the given name.
/// The value is initialized by calling the init_profiling function from the
/// profiling library.
/// This call is added to module constructors 'llvm.global_ctors'
llvm::GlobalVariable* createGlobalProfilingContextValue(llvm::Module& M);

/// This creates a call to finish_profiling function from the profiling library.
/// This call is added to module destructors 'llvm.global_dtors'
void createFinishProfilingCall(llvm::Module& M,
                               llvm::GlobalVariable* ProfilingContext);

} // namespace codegen
} // namespace pollyML
