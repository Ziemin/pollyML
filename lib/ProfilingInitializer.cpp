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

static cl::opt<std::string>
    ProfilingJsonOutputFile("pollyML-profiling-json-out-path",
                            cl::desc("The path to save the json file with profiling results."),
                            cl::Hidden, cl::value_desc("file path"), cl::ValueRequired,
                            cl::init("stats.json"), cl::cat(PollyMLCategory));

static cl::opt<std::string>
    ProfilingConfigFile("pollyML-config-json-file",
                         cl::desc("Path to json with the configuration of"
                                  " the profiler"),
                         cl::Hidden, cl::ValueRequired,
                         cl::init("profiler_config.json"),
                         cl::cat(PollyMLCategory));

char ProfilingInitializer::ID = 0;

// We should not inject more calls to profiling library than just one
bool ProfilingInitializer::initialized_profiler = false;

bool ProfilingInitializer::runOnModule(Module &M) {
  if (initialized_profiler) {
    DEBUG(errs() << "Profiling has already been initialized\n");
    return false;
  }

  DEBUG(errs() << "Injecting ScopProfiling initialization and finalization code to module: "
               << M.getName() << '\n');

  GlobalVariable *profilingContext = codegen::createGlobalProfilingContextValue(
      M, ProfilingConfigFile);
  codegen::createFinishProfilingCall(M, profilingContext, ProfilingJsonOutputFile);
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
