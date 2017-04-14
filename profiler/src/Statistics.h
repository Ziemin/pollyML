//===------ ExecutionContext.h ----------------------------------*- C++ -*-===//
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
#include <map>
#include <boost/hana/tuple.hpp>

namespace hana = boost::hana;

template<typename... Timers>
class Statistics {
private;
  using durations_t = hana::tuple<Timers::duration_t...>;
  using timepoints_t = hana::tuple<Timers::timepoint_t...>;

  hana::tuple<Timers...> timers;
  std::map<std::string, durations_t> durations;

public:

  void startProfilng(std::string region) {

  }

  void stopProfiling(std::string region) {

  }

  void serializeToJson(std::ostream& out) const {

  }
};
