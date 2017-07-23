//===------ Timers.h --------------------------------------------*- C++ -*-===//
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

#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <boost/chrono.hpp>
#include <iostream>
#include <string_view>
#include <vector>
#include <papi.h>

#include "scop_profiler/Papi.h"

namespace chr = boost::chrono;

//===- Timer contract ----------------------------------------------------------
//
//  struct Timer {
//     name;
//     timepoint_t;
//     duration_t;
//     now() -> timepoint_t;
//  }
//
//  Timer::timepoint_t - Timer::timepoint_t -> Timer::duration_t
//  operator<<(ostream, duration) -> ostream
//
//===---------------------------------------------------------------------------

/// Timer RTDSC instruction to count CPU cycles
struct RDTSCTimer {
  static std::string_view name;

  using timepoint_t = uint64_t;
  using duration_t = uint64_t;
  uint64_t time;

  static auto now() -> timepoint_t {
    uint32_t lo, hi;
    asm volatile ("rdtscp" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
  }

  auto print(std::ostream& out, uint64_t dur) const -> void;
};


/// TODO description
struct ClockTimer {
  static std::string_view name;

  timespec time;

  using duration_t = timespec;
  using timepoint_t = timespec;

  static auto now() -> timepoint_t {
    timespec tm;
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tm);
    return tm;
  }

  auto print(std::ostream& out, const duration_t& dur) const -> void;
};

auto operator-(ClockTimer::timepoint_t end_time,
               ClockTimer::timepoint_t beg_time) -> ClockTimer::duration_t;


/// Timer using boost's cpu user clock to measure user time.
struct BoostUserTimer {
  static std::string_view name;

  using clock_t = chr::process_user_cpu_clock;

  using duration_t = clock_t::duration;
  using timepoint_t = chr::time_point<clock_t>;

  static auto now() -> timepoint_t {
    return clock_t::now();
  }

  auto print(std::ostream& out, const duration_t& dur) const -> void;
};


/// Timer collecting several hardware counters using PAPI.
struct PAPITimer {
  static std::string_view name;

  using timepoint_t = std::vector<long long int>;
  using duration_t = std::vector<long long int>;

  PAPITimer(papi::EventSet_t event_set);

  auto now() -> timepoint_t;

  auto print(std::ostream& out, const duration_t& dur) const -> void;

private:
  papi::EventSet_t event_set;
  int num_events;
  std::vector<long long int> counters;
  int event_codes[10];
};

auto operator-(PAPITimer::timepoint_t end_time,
               PAPITimer::timepoint_t beg_time) -> PAPITimer::duration_t;
