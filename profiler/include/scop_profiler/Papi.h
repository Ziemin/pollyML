//===------ Papi.h ----------------------------------------------*- C++ -*-===//
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

#include <vector>
#include <string>

namespace papi {

  using EventSet_t = int;

  auto init_papi(const std::vector<std::string>& papi_events) -> EventSet_t;

} // namesapce papi
