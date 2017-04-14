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
#include "llvm/IR/IRBuilder.h"

namespace pollyML {
namespace codegen {

void createPrintCall(llvm::IRBuilder<> &Builder, llvm::StringRef Text);

} // namespace codegen
} // namespace pollyML
