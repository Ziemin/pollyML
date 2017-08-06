//===------ RegisterPasses.cpp - Add the PollyML Passes to default passes
//--===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//===----------------------------------------------------------------------===//

// TODO - we want to inject pollyML optimization passes somewhere in
// between polly passes depending on the provided configuration.

#include "pollyML/RegisterPasses.h"
#include "pollyML/ScopProfiling.h"
#include "pollyML/ProfilingInitializer.h"
#include "pollyML/Options.h"

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

using namespace llvm;
using namespace polly;
using namespace pollyML;

cl::OptionCategory PollyMLCategory("PollyML Options",
                                   "Configure the pollyML loop optimizer"
                                   "and profiler");

namespace pollyML {
void initializePollyMLPasses(PassRegistry &Registry) {
  initializeProfilingInitializerPass(Registry);
  initializeScopProfilingPass(Registry);
}

static void registerPollyMLPasses(const llvm::PassManagerBuilder &Builder,
                      llvm::legacy::PassManagerBase &PM) {

  PM.add(pollyML::createProfilingInitializerPass());
  PM.add(pollyML::createScopProfilingPass());
}

static llvm::RegisterStandardPasses RegisterPollyMLPasses(
    llvm::PassManagerBuilder::EP_ModuleOptimizerEarly,
    registerPollyMLPasses);

} // namespace pollyML
