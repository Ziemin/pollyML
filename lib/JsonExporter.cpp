//===------- pollyML/JsonExporter.cpp - Scop Info Exporter  ---- *- C++ -*-===//
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

#include "isl/ctx.h"
#include "polly/DependenceInfo.h"
#include "polly/Support/GICHelper.h"
#include "pollyML/JsonExporter.h"
#include "pollyML/Options.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/FormatVariadic.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Support/raw_ostream.h"

#define JSON_NOEXCEPTION
#include <nlohmann/json.hpp>

using namespace llvm;
using namespace polly;
using namespace pollyML;

#define DEBUG_TYPE "pollyML-export-jscop"

using json = nlohmann::json;

namespace {
static cl::opt<std::string>
    ExportDir("pollyML-export-scopson-dir",
              cl::desc("The directory to export the scopson files to."),
              cl::Hidden, cl::value_desc("Directory path"), cl::ValueRequired,
              cl::init("."), cl::cat(PollyMLCategory));


json exportArrays(const Scop &S) {
  json arrays;
  std::string buffer;
  llvm::raw_string_ostream rawStringOstream(buffer);

  for (auto &SAI : S.arrays()) {
    std::vector<std::string> sizes;
    if (SAI->isArrayKind()) {
      unsigned i = 0;
      if (!SAI->getDimensionSize(i)) {
        sizes.push_back("*");
        i++;
      }
      for (; i < SAI->getNumberOfDimensions(); i++) {
        SAI->getDimensionSize(i)->print(rawStringOstream);
        sizes.push_back(rawStringOstream.str());
        buffer.clear();
      }
    }
    SAI->getElementType()->print(rawStringOstream);
    auto arrType = rawStringOstream.str();
    buffer.clear();

    arrays.push_back(
      {
       {"name", SAI->getName()},
       {"sizes", std::move(sizes)},
       {"type", std::move(arrType)}
      }
    );
  }
  return arrays;
}


json exportDependences(const Dependences &D) {
  isl::union_map RAW = isl::manage(D.getDependences(Dependences::TYPE_RAW));
  isl::union_map WAR = isl::manage(D.getDependences(Dependences::TYPE_WAR));
  isl::union_map WAW = isl::manage(D.getDependences(Dependences::TYPE_WAW));
  isl::union_map RED = isl::manage(D.getDependences(Dependences::TYPE_RED));
  isl::union_map TC_RED = isl::manage(D.getDependences(Dependences::TYPE_TC_RED));
  return {
    {"RAW", RAW.to_str()},
    {"WAR", WAR.to_str()},
    {"WAW", WAW.to_str()},
    {"RED", RED.to_str()},
    {"TC_RED", TC_RED.to_str()}
  };
}


json getJson(const Scop &S, const Dependences &D) {

  json statements;
  for (const auto &Stmt : S) {
    json accesses;
    for (MemoryAccess *MA : Stmt) {
      json access = {
        {"kind", MA->isRead() ? "read" : "write"},
        {"relation", MA->getAccessRelationStr()}
      };
      accesses.push_back(std::move(access));
    }

    statements.push_back(
      {
        {"name", Stmt.getBaseName()},
        {"domain", Stmt.getDomainStr()},
        {"schedule", Stmt.getScheduleStr()},
        {"accesses", std::move(accesses)}
      });
  }

  return {
    {"name", S.getNameStr()},
    {"context", S.getContextStr()},
    {"arrays", exportArrays(S)},
    {"statements", std::move(statements)},
    {"dependences", exportDependences(D)}
  };
}

void exportScopson(const Scop &S, const Dependences &D,
                   const std::string &FileName) {
  auto scopson = getJson(S, D);

  errs() << formatv("Writing SCoPSON {0} to {1}\n", S.getNameStr(), FileName);

  std::error_code EC;
  ToolOutputFile F(FileName, EC, llvm::sys::fs::F_Text);
  if (!EC) {
    F.os() << scopson.dump(4);
    F.os().close();
    if (!F.os().has_error()) {
      errs() << "\n";
      F.keep();
      return;
    }
  }

  errs() << "  error opening file for writing!\n";
  F.os().clear_error();
}
} // namespace

char ScopJsonExporter::ID = 0;

void ScopJsonExporter::getAnalysisUsage(llvm::AnalysisUsage &AU) const {
  AU.addRequiredTransitive<ScopInfoRegionPass>();
  AU.addRequiredTransitive<DependenceInfo>();
  AU.setPreservesAll();
}

void ScopJsonExporter::printScop(llvm::raw_ostream &OS, polly::Scop &S) const {
  OS << S;
}

bool ScopJsonExporter::runOnScop(polly::Scop &S) {
  const auto &D =
      getAnalysis<DependenceInfo>().getDependences(Dependences::AL_Statement);

  std::string FileName = formatv("{0}/{1}___{2}.scopson", ExportDir,
                                 S.getFunction().getName(), S.getNameStr());

  exportScopson(S, D, FileName);
  return false;
}

Pass *pollyML::createScopJsonExporterPass() { return new ScopJsonExporter(); }

INITIALIZE_PASS_BEGIN(ScopJsonExporter, "pollyML-export-scopson",
                      "PollyML - Export extended SCoP as extended jscop", false,
                      true)
INITIALIZE_PASS_DEPENDENCY(ScopInfoRegionPass);
INITIALIZE_PASS_DEPENDENCY(DependenceInfo);
INITIALIZE_PASS_END(ScopJsonExporter, "pollyML-export-scopson",
                    "PollyML - Export extended SCoP as extended jscop", false,
                    true)
