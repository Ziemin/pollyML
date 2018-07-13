//===------- pollyML/JsonExporter.h - Scop Info Exporter  ----- *- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Exports information about the SCoP
//
//===----------------------------------------------------------------------===//

#pragma once


#include "polly/ScopPass.h"

namespace pollyML {

  class ScopJsonExporter : public polly::ScopPass {

  public:
    static char ID;

    ScopJsonExporter() : polly::ScopPass(ID) {}

    /// Export the SCoP @p S to JSON file
    bool runOnScop(polly::Scop &S) override;

    void getAnalysisUsage(llvm::AnalysisUsage &AU) const override;

    void printScop(llvm::raw_ostream &OS, polly::Scop &S) const override;

  };

  llvm::Pass *createScopJsonExporterPass();

} // namespace pollyML

namespace llvm {
  class PassRegistry;
  void initializeScopJsonExporterPass(llvm::PassRegistry &);
} // namespace llvm
