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

#include "polly/RegisterPasses.h"
#include "polly/Canonicalization.h"
#include "polly/CodeGen/CodeGeneration.h"
#include "polly/CodeGen/CodegenCleanup.h"
#include "polly/CodeGen/PPCGCodeGeneration.h"
#include "polly/DeLICM.h"
#include "polly/DependenceInfo.h"
#include "polly/FlattenSchedule.h"
#include "polly/LinkAllPasses.h"
#include "polly/Options.h"
#include "polly/PolyhedralInfo.h"
#include "polly/ScopDetection.h"
#include "polly/ScopInfo.h"
#include "polly/Simplify.h"
#include "polly/Support/DumpModulePass.h"

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Transforms/IPO.h"

using namespace llvm;
using namespace polly;
using namespace pollyML;

cl::OptionCategory PollyMLCategory("PollyML Options",
                                   "Configure the pollyML loop optimizer");

static cl::opt<bool>
    UseAfterScheduleOptimizer("pollyML-after-optimizer",
                              cl::desc("Whether profiler should be used after "
                                       " the schedule optimizer."),
                              cl::Hidden, cl::ValueRequired,
                              cl::init(false),
                              cl::cat(PollyMLCategory));


static cl::opt<bool>
    PollyMLProfiler("pollyML-profiler",
                    cl::desc("Enable the pollyML profiler"),
                    cl::init(false),
                    cl::ZeroOrMore,
                    cl::cat(PollyMLCategory));

namespace pollyML {
void initializePollyMLPasses(PassRegistry &Registry) {
  initializeCodeGenerationPass(Registry);

  initializeProfilingInitializerPass(Registry);
  initializeScopProfilingPass(Registry);

  initializeCodePreparationPass(Registry);
  initializeDeadCodeElimPass(Registry);
  initializeDependenceInfoPass(Registry);
  initializeDependenceInfoWrapperPassPass(Registry);
  initializeJSONExporterPass(Registry);
  initializeJSONImporterPass(Registry);
  initializeIslAstInfoWrapperPassPass(Registry);
  initializeIslScheduleOptimizerPass(Registry);
  initializePollyCanonicalizePass(Registry);
  initializePolyhedralInfoPass(Registry);
  initializeScopDetectionWrapperPassPass(Registry);
  initializeScopInfoRegionPassPass(Registry);
  initializeScopInfoWrapperPassPass(Registry);
  initializeCodegenCleanupPass(Registry);
  initializeFlattenSchedulePass(Registry);
  initializeDeLICMPass(Registry);
  initializeSimplifyPass(Registry);
  initializeDumpModulePass(Registry);
  initializePruneUnprofitablePass(Registry);
}

static void registerPollyMLPasses(
    const llvm::PassManagerBuilder &Builder,
    llvm::legacy::PassManagerBase &PM) {

  if (!PollyMLProfiler)
    return;

  PM.add(pollyML::createProfilingInitializerPass());

  polly::registerCanonicalicationPasses(PM);

  PM.add(polly::createScopDetectionWrapperPassPass());
  PM.add(polly::createPruneUnprofitablePass());

  PM.add(pollyML::createScopProfilingPass());

  PM.add(polly::createIslScheduleOptimizerPass());
  PM.add(polly::createCodeGenerationPass());
  PM.add(llvm::createBarrierNoopPass());
}

static llvm::RegisterStandardPasses RegisterPollyMLPasses(
    llvm::PassManagerBuilder::EP_ModuleOptimizerEarly,
    registerPollyMLPasses);

} // namespace pollyML
