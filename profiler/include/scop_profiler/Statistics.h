//===------ Statistics.h ----------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include <iostream>
#include <vector>
#include <utility>
#include <nlohmann/json.hpp>
#include <boost/hana/tuple.hpp>
#include <boost/hana/transform.hpp>
#include <boost/hana/zip_with.hpp>
#include <boost/hana/minus.hpp>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringMap.h>
#include <llvm/ADT/StringRef.h>
#include <scop_profiler/Utils.h>

namespace hana = boost::hana;
using json = nlohmann::json;

using params_t = std::vector<hana::pair<std::string_view, int64_t>>;

/// This is a class used to store execution results of different timers for
/// selected SCoPs. It is destined to be used in single-threaded environment.
template<typename... Timers>
class Statistics {

private:
  using durations_t = hana::tuple<typename Timers::duration_t...>;
  using timepoints_t = hana::tuple<typename Timers::timepoint_t...>;
  using timers_t = hana::tuple<Timers...>;

  // Last measured times. We are assuming here that SCoPs cannot contain
  // calls to functions with other SCoPs.
  timepoints_t last_times;

  timers_t timers;
  // Each SSoP execution should contain durations for all timers
  // and values of their execution parameters.
  llvm::StringMap<std::vector<durations_t>> durations;
  llvm::StringMap<std::vector<params_t>> parameters;

  auto getCurrentTimepoints() -> timepoints_t {
    return hana::transform(
        timers,
        [](auto& timer){ return timer.now(); });
  }

public:

  Statistics() = default;

  Statistics(Timers... timers)
    : timers(hana::make_tuple(std::move(timers)...)) { }

  /// Starts timers for the chosen region and saves values of the parameters.
  auto startProfiling(llvm::StringRef region, params_t scop_params) -> void {

    DEBUG_PRINT(
      std::cerr << "Start of SCoP: " << region.data() << '\n';
      for (auto& p : scop_params) {
        std::cerr << hana::first(p) << " " << hana::second(p) << '\n';
      }
    );
    last_times = getCurrentTimepoints();

    parameters[region].push_back(std::move(scop_params));
  }

  /// Stops timers for the chosen region and saves values of the parameters.
  auto stopProfiling(llvm::StringRef region) -> void {
    auto new_times = getCurrentTimepoints();
    auto scop_durations = hana::zip_with(
        [](const auto& end_t, const auto& beg_t) { return end_t - beg_t; },
        new_times,
        last_times);

    DEBUG_PRINT(
      std::cerr << "End of SCoP: " << region.data() << '\n'
                  << "Timer results: \n";
      hana::zip_with(
          [](const auto& timer, const auto& dur) {
            std::cerr << "-- " << timer.name.data() << ": ";
            timer.print(std::cerr, dur);
            std::cerr << '\n';
            return 1;
          },
          timers, scop_durations);
    );

    durations[region].push_back(std::move(scop_durations));
  }

  /// Saves the results to json file
  ///
  /// @param out_path  The path to store the results.
  auto serializeToJson() const -> json {
    json results;

    for (auto& scop_entry : durations) {
      llvm::StringRef scop_name = scop_entry.getKey();

      const auto& scop_durations = scop_entry.getValue();
      const auto& scop_params = parameters.lookup(scop_name);

      json scop_results = json::array();
      json parameter_names = json::array();

      const size_t count = scop_durations.size();
      for (size_t i = 0; i < count; i++) {
        // set parameter names for SCOP
        if (i == 0) {
          for (const auto& param: scop_params[0])
            parameter_names.push_back(hana::first(param).data());
        }

        // save parameter values
        json parameter_values = json::array();
        for (const auto& param: scop_params[i])
          parameter_values.push_back(hana::second(param));

        // save timer results
        json timers_results;
        hana::zip_with(
            [&timers_results](const auto& timer, const auto& dur) {
              timer.to_json(timers_results[timer.name.data()], dur);
              return 1;
            },
            timers, scop_durations[i]);

        scop_results.push_back({
            {"parameters", std::move(parameter_values)},
            {"timers", std::move(timers_results)}
        });
      }

      results[scop_name] = {
        {"parameter_names", std::move(parameter_names)},
        {"results", std::move(scop_results)}
      };
    }

    return results;
  }
};
