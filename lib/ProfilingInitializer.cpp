//===------- pollyML/ProfilingInitializer.cpp ------------------ *- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Inject profiling library initialization.
//
//===----------------------------------------------------------------------===//


#include "pollyML/ProfilingInitializer.h"
#include "pollyML/ProfilingCodegen.h"
#include "pollyML/Options.h"

#include "llvm/IR/Module.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/PassSupport.h"
#include "llvm/Pass.h"

using namespace llvm;
using namespace pollyML;

#define DEBUG_TYPE "ProfilingInitializer"

char ProfilingInitializer::ID = 0;

// We should not inject more calls to profiling library than just one
bool ProfilingInitializer::initialized_profiler = false;

bool ProfilingInitializer::runOnModule(Module &M) {
  if (initialized_profiler) {
    DEBUG(errs() << "Profiling has already been initialized\n");
    return false;
  }

  DEBUG(errs() << "Injecting ScopProfiling functions to module: "
               << M.getName() << '\n');

  codegen::createInitProfilingCall(M);
  codegen::createFinishProfilingCall(M);
  codegen::createStartAndStopProfilingDeclarations(M);

  initialized_profiler = true;

  return true;
}


Pass *pollyML::createProfilingInitializerPass() {
  return new ProfilingInitializer();
}

INITIALIZE_PASS(ProfilingInitializer, "pollyML-profiling-initializer",
                "PollyML - Inject profiling initialization code to modules.",
                false, false)
