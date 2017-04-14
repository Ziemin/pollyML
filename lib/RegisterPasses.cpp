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

using namespace llvm;
using namespace polly;
using namespace pollyML;

namespace pollyML {

void initializePollyMLPasses(PassRegistry &Registry) {
  initializeScopProfilingPass(Registry);
}
void registerPollyMLPasses(legacy::PassManagerBase &PM) {}

} // namespace pollyML
