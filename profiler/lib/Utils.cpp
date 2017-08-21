//===------ Utils.cpp -------------------------------------------*- C++ -*-===//
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

#include "scop_profiler/Utils.h"

auto flush_cache(int cache_size) -> void {
  int to_flush = 1024 * cache_size / sizeof(double);
  double *some_memory = new double[to_flush]();
  double sum = 0;
  for (int i = 0; i < to_flush; i++) {
    sum += some_memory[i];
  }
  delete [] some_memory;
}
