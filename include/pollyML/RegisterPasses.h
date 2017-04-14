//===------ pollyML/RegisterPasses.h - Register the Polly passes *- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Functions to register the PollyML passes in a LLVM pass manager.
//
//===----------------------------------------------------------------------===//
#pragma once

#include "llvm/IR/LegacyPassManager.h"

namespace llvm {
namespace legacy {
class PassManagerBase;
} // namespace legacy
} // namespace llvm

namespace pollyML {
void initializePollyMLPasses(llvm::PassRegistry &Registry);
void registerPollyMLPasses(llvm::legacy::PassManagerBase &PM);
} // namespace pollyML
