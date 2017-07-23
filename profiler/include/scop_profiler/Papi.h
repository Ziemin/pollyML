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

namespace papi {

  using EventSet_t = int;

  auto init_papi(int events_num, const char** event_code_names) -> EventSet_t;

} // namesapce papi
