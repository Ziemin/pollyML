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
#include <iostream>
#include <fstream>
#include <stdint.h>
#include <utility>
#include <nlohmann/json.hpp>

#include "scop_profiler/Papi.h"
#include "scop_profiler/Statistics.h"
#include "scop_profiler/Timers.h"

using ScopStatistics =
    Statistics<RDTSCTimer, ClockTimer, BoostUserTimer, PAPITimer>;

/// Start profiling the program
///
//  @param num_papi_events  The number of PAPI counters to follow
//  @param papi_event_names The array with `num_papi_events` strings with the
//                          names of the events to follow.
/// @return pointer to a context handle of particular execution
extern "C" void *init_profiling(int num_papi_events,
                                const char *papi_event_names[]) {
  // TODO get settings
  std::cerr << "Profiling initialization" << std::endl;

  // setting up PAPI library
  papi::EventSet_t event_set =
      papi::init_papi(num_papi_events, papi_event_names);

  auto *stats = new ScopStatistics(RDTSCTimer(), ClockTimer(), BoostUserTimer(),
                            PAPITimer(event_set));
  // to measure raw time of callng timers
  stats->startProfiling("EMPTY", {});
  stats->stopProfiling("EMPTY");

  return stats;
}

/// Start timers for the given SCOP.
///
/// @param context          The pointer to a context handle of particular
/// execution.
/// @param region_name      The name of the region being profiled.
/// @param param_count      The number of parameters for this ScoP.
/// @param parameter_names  The names of SCoP parameters.
/// @param parameter_values The values of the SCoP parameters during execution.
extern "C" void start_scop(void *context, char *region_name, int param_count,
                           const char *parameter_names[],
                           int64_t parameter_values[]) {

  ScopStatistics *stats = reinterpret_cast<ScopStatistics *>(context);
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
extern "C" void stop_scop(void *context, char *region_name) {
  ScopStatistics *stats = reinterpret_cast<ScopStatistics *>(context);
  stats->stopProfiling(region_name);
}

/// Stop the profiler. This should result in saving profiling results.
///
/// @param context  The pointer to a context handle of particular execution
/// @param output_file  The string with the path of the destination file.
extern "C" void finish_profiling(void *context, const char *output_file) {
  // TODO serialization of the results
  ScopStatistics *stats = reinterpret_cast<ScopStatistics *>(context);
  nlohmann::json data = stats->serializeToJson();

  if (std::ofstream os(output_file); os) {
    os << std::setw(4) << data << std::endl;
  } else {
    std::cerr << "Could not write to " << output_file << std::endl;
    exit(1);
  }

  delete stats;
}
