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
#include <cstdlib>
#include <string_view>
#include <nlohmann/json.hpp>
#include <memory>

#include "scop_profiler/Papi.h"
#include "scop_profiler/Statistics.h"
#include "scop_profiler/Timers.h"
#include "scop_profiler/Utils.h"

using json = nlohmann::json;

using ScopStatistics =
    Statistics<RDTSCTimer, ClockTimer, PAPITimer>;


namespace {
struct ProfilingContext {
  ProfilingContext();
  ~ProfilingContext();

  bool flush_cache;  // if cache should be flushed on every scop start
  int cache_size;    // the size of the cache to flush (in KB)
  std::unique_ptr<ScopStatistics> statistics;  // handle to the stats gatherer

};

std::unique_ptr<ProfilingContext> ctx;

constexpr const char* DEFAULT_CONFIG = "profiler_config.json";
constexpr const char* DEFAULT_OUTPUT = "profiler_stats.json";

} // anonymous namespace


/// Start profiling the program
///
/// @param configuration_file Path to a file with profiler configuration

/// @return pointer to a context handle of particular execution
ProfilingContext::ProfilingContext()
  : flush_cache(false)
  , cache_size(0)
{
  DEBUG_PRINT(std::cerr << "Profiling initialization" << std::endl);

  // read config file
  const char* configuration_file_env = std::getenv("SCOP_PROFILER_CONFIG");
  std::string_view configuration_file;
  if (configuration_file_env == nullptr) {
    configuration_file = DEFAULT_CONFIG;
  } else {
    configuration_file = configuration_file_env;
  }
  std::ifstream is(configuration_file.data());
  if (!is) {
    std::cerr << "Could not open config file " << configuration_file << std::endl;
    exit(1);
  }
  json config;
  is >> config;
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

  statistics = std::make_unique<ScopStatistics>(
      RDTSCTimer(), ClockTimer(), PAPITimer(event_set));

  // to measure raw time of calling timers
  for (int i = 0; i < 5; i++) {
    statistics->startProfiling("EMPTY", {});
    statistics->stopProfiling("EMPTY");
  }
}

extern "C" void init_profiling() {
  if (!ctx) {
    ctx = std::make_unique<ProfilingContext>();
  }
}
/// Start timers for the given SCOP.
///
/// @param context          The pointer to a context handle of particular
/// execution.
/// @param region_name      The name of the region being profiled.
/// @param param_count      The number of parameters for this ScoP.
/// @param parameter_names  The names of SCoP parameters.
/// @param parameter_values The values of the SCoP parameters during execution.
extern "C" void start_scop(char *region_name, int param_count,
                           const char *parameter_names[],
                           int64_t parameter_values[]) {
  params_t parameters;
  for (int i = 0; i < param_count; i++) {
    parameters.emplace_back(parameter_names[i], parameter_values[i]);
  }
  if (ctx->flush_cache) {
    DEBUG_PRINT(
      std::cerr << "Flushing cache of size " << ctx->cache_size << " KB \n");
    flush_cache(ctx->cache_size);
  }
  ctx->statistics->startProfiling(region_name, std::move(parameters));
}

/// Stop timers for the given SCOP.
///
/// @param region_name  The name of the region being profiled
extern "C" void stop_scop(char *region_name) {
  ctx->statistics->stopProfiling(region_name);
}

/// Stop the profiler. This should result in saving profiling results.
///
/// @param context  The pointer to a context handle of particular execution
/// @param output_file  The string with the path of the destination file.
ProfilingContext::~ProfilingContext() {

  const char* profiling_output_env = std::getenv("SCOP_PROFILING_OUTPUT");
  std::string_view profiling_output;
  if (profiling_output_env == nullptr) {
    profiling_output = DEFAULT_OUTPUT;
  } else {
    profiling_output = profiling_output_env;
  }
  json data = statistics->serializeToJson();

  if (std::ofstream os(profiling_output.data()); os) {
    os << std::setw(4) << data << std::endl;
  } else {
    std::cerr << "Could not write to " << profiling_output << std::endl;
    exit(1);
  }
}

extern "C" void finish_profiling() {
  if (ctx) {
    ctx.reset();
  }
}
