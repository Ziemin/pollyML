//===--- Constants.h ------------------------------------------ LLVM-IR ---===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// These are the common constants used in the library
//
//===----------------------------------------------------------------------===//

#pragma once

namespace pollyML {

constexpr const char* PROFILING_CONTEXT_VAR_NAME = "__profiling_ctx";
constexpr const char* INIT_PROFILING_FUN_NAME = "init_profiling";
constexpr const char* FINISH_PROFILING_FUN_NAME = "finish_profiling";
constexpr const char* START_SCOP_FUN_NAME = "start_scop";
constexpr const char* STOP_SCOP_FUN_NAME = "stop_scop";

} // namespace pollyML
