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

/// Start profiling of the program
///
/// @return pointer to a context handle of particular execution
extern "C" void* init_profiling();

/// Returns an allocated object responsible for storing parameters values.
extern "C" void* new_parameters();

/// Adds a parameter value to parameters container
extern "C" void add_parameter(void* parameters, const char* name, bool value);
extern "C" void add_parameter(void* parameters, const char* name, char value);
extern "C" void add_parameter(void* parameters, const char* name, int value);
extern "C" void add_parameter(void* parameters, const char* name, int64_t value);

/// Start timers for the given SCOP.
///
/// @param context  The pointer to a context handle of particular execution
/// @param paramters The pointer to a struct containing parameter values
/// @param region_name  The name of the region being profiled
extern "C" void start_scop(void* context, void* parameters, char* region_name);

/// Stop timers for the given SCOP.
///
/// @param context      The pointer to a context handle of particular execution
/// @param region_name  The name of the region being profiled
extern "C" void stop_scop(void* context, char* region_name);

/// Stop the profiler. This should result in saving profiling results.
///
/// @param context  The pointer to a context handle of particular execution
extern "C" void finish_profiling(void* context);
