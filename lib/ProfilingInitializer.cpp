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
#include "llvm/IR/Module.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/PassSupport.h"
#include "llvm/Pass.h"

using namespace llvm;
using namespace pollyML;

#define DEBUG_TYPE "ProfilingInitializer"

char ProfilingInitializer::ID = 0;

bool ProfilingInitializer::runOnModule(Module &M) {
  DEBUG(errs() << "Injecting ScopProfiling initialization and finalization code to module: "
               << M.getName() << '\n');

  GlobalVariable *profilingContext = codegen::createGlobalProfilingContextValue(M);
  codegen::createFinishProfilingCall(M, profilingContext);
  codegen::createStartAndStopProfilingDeclarations(M);

  return true;
}


Pass *pollyML::createProfilingInitializerPass() {
  return new ProfilingInitializer();
}

INITIALIZE_PASS(ProfilingInitializer, "pollyML-profiling-initializer",
                "PollyML - Inject profiling initialization code to modules.",
                false, false)
