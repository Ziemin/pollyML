//===------ profiling.cpp.h -------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the whole API of the profiling library.
// It contains functions, which can be called from the profiled code.
//
//===----------------------------------------------------------------------===//
#include <stdint.h>
#include <utility>
#include <iostream>

#include "scop_profiler/Timers.h"
#include "scop_profiler/Statistics.h"

using ScopStatistics = Statistics<RDTSCTimer, ClockTimer, BoostUserTimer>;

/// Start profiling the program
///
/// @return pointer to a context handle of particular execution
extern "C" void* init_profiling() {
  // TODO get settings
  std::cerr << "Profiling initialization" << std::endl;
  return new ScopStatistics();
}

/// Start timers for the given SCOP.
///
/// @param context          The pointer to a context handle of particular execution.
/// @param region_name      The name of the region being profiled.
/// @param param_count      The number of parameters for this ScoP.
/// @param parameter_names  The names of SCoP parameters.
/// @param parameter_values The values of the SCoP parameters during execution.
extern "C" void start_scop(void* context, char* region_name,
                           int param_count, const char* parameter_names[],
                           int64_t parameter_values[]) {

  ScopStatistics* stats = reinterpret_cast<ScopStatistics*>(context);
  params_t parameters;
  for (int i = 0; i < param_count; i++) {
    parameters.emplace_back(parameter_names[i], parameter_values[i]);
  }
  stats->startProfiling(region_name, std::move(parameters));
}

/// Stop timers for the given SCOP.
///
/// @param context      The pointer to a context handle of particular execution
/// @param region_name  The name of the region being profiled
extern "C" void stop_scop(void* context, char* region_name) {
  ScopStatistics* stats = reinterpret_cast<ScopStatistics*>(context);
  stats->stopProfiling(region_name);
}

/// Stop the profiler. This should result in saving profiling results.
///
/// @param context  The pointer to a context handle of particular execution
extern "C" void finish_profiling(void* context) {
  // TODO serialization of the results
  ScopStatistics* stats = reinterpret_cast<ScopStatistics*>(context);
  stats->serializeToJson("/");
}
