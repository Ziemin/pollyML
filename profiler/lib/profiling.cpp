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
#include <vector>
#include <nlohmann/json.hpp>
#include <memory>

#include "scop_profiler/Papi.h"
#include "scop_profiler/Statistics.h"
#include "scop_profiler/Timers.h"
#include "scop_profiler/Utils.h"

using json = nlohmann::json;

using ScopStatistics =
    Statistics<RDTSCTimer, ClockTimer, BoostUserTimer, PAPITimer>;


struct ProfilingContext {
  bool flush_cache;  // if cache should be flushed on every scop start
  int cache_size;    // the size of the cache to flush (in KB)
  std::unique_ptr<ScopStatistics> statistics;  // handle to the stats gatherer
};

/// Start profiling the program
///
/// @param configuration_file Path to a file with profiler configuration

/// @return pointer to a context handle of particular execution
extern "C" void *init_profiling(const char *configuration_file) {
  DEBUG_PRINT(std::cerr << "Profiling initialization" << std::endl);

  // read config file
  std::ifstream is(configuration_file);
  if (!is) {
    std::cerr << "Could not open config file " << configuration_file << std::endl;
    exit(1);
  }
  json config;
  is >> config;
  int cache_size = 0;
  bool flush_cache = false;
  if (config.find("flush_cache") != end(config) && config["flush_cache"].get<bool>()) {
    flush_cache = true;
    if (config.find("cache_size") == end(config)) {
      std::cerr << "Cache size should be provided when flushing" << std::endl;
    }
    cache_size = config["cache_size"];
  }

  if (config.find("papi_events") == end(config)) {
    std::cerr << "No papi_events field in the config" << std::endl;
    exit(1);
  }
  std::vector<std::string> papi_events = config["papi_events"];
  // setting up PAPI library
  papi::EventSet_t event_set = papi::init_papi(papi_events);

  auto *stats = new ScopStatistics(RDTSCTimer(), ClockTimer(), BoostUserTimer(),
                            PAPITimer(event_set));
  // to measure raw time of callng timers
  stats->startProfiling("EMPTY", {});
  stats->stopProfiling("EMPTY");

  return new ProfilingContext{
    flush_cache,
    cache_size,
    std::unique_ptr<ScopStatistics>(stats)
  };
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

  ProfilingContext *ctx = reinterpret_cast<ProfilingContext *>(context);
  params_t parameters;
  for (int i = 0; i < param_count; i++) {
    parameters.emplace_back(parameter_names[i], parameter_values[i]);
  }
  ctx->statistics->startProfiling(region_name, std::move(parameters));
}

/// Stop timers for the given SCOP.
///
/// @param context      The pointer to a context handle of particular execution
/// @param region_name  The name of the region being profiled
extern "C" void stop_scop(void *context, char *region_name) {
  ProfilingContext *ctx = reinterpret_cast<ProfilingContext *>(context);
  if (ctx->flush_cache) {
    DEBUG_PRINT(
      std::cerr << "Flushing cache of size " << ctx->cache_size << " KB \n");
    flush_cache(ctx->cache_size);
  }
  ctx->statistics->stopProfiling(region_name);
}

/// Stop the profiler. This should result in saving profiling results.
///
/// @param context  The pointer to a context handle of particular execution
/// @param output_file  The string with the path of the destination file.
extern "C" void finish_profiling(void *context, const char *output_file) {

  ProfilingContext *ctx = reinterpret_cast<ProfilingContext *>(context);
  json data = ctx->statistics->serializeToJson();

  if (std::ofstream os(output_file); os) {
    os << std::setw(4) << data << std::endl;
  } else {
    std::cerr << "Could not write to " << output_file << std::endl;
    exit(1);
  }

  delete ctx;
}
