//===------- pollyML/ProfilingInitializer.h -------------------- *- C++ -*-===//
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

#include "llvm/Pass.h"

namespace llvm {
  class Module;
  class GlobalVariable;
} // namespace llvm

namespace pollyML {

class ProfilingInitializer : public llvm::ModulePass {

public:
  static char ID;

  ProfilingInitializer() : llvm::ModulePass(ID), profilingContext(nullptr) { }

  bool runOnModule(llvm::Module &M) override;

  llvm::GlobalVariable *getProfilingContextValue() {
    return profilingContext;
  }

private:
  llvm::GlobalVariable *profilingContext;

};

llvm::Pass *createProfilingInitializerPass();

} // namespace pollyML

namespace llvm {
class PassRegistry;
void initializeProfilingInitializerPass(llvm::PassRegistry &);
}
