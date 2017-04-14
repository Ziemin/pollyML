//===------ pollyML/PollyML.cpp -------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// PollyML and Polly initialization
//
//===----------------------------------------------------------------------===//

#include "pollyML/RegisterPasses.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

namespace {

class StaticInitializer {
public:
  StaticInitializer() {
    llvm::PassRegistry &Registry = *llvm::PassRegistry::getPassRegistry();
    pollyML::initializePollyMLPasses(Registry);
  }
};
static StaticInitializer InitializeEverything;

} // anonymous namespace
