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

//===- Timer contract ----------------------------------------------------------
//
//  struct Timer {
//     timepoint_t;
//     duration_t;
//     now() -> timepoint_t;
//  }
//
//  Timer::timepoint_t - Timer::timepoint_t -> Timer::duration_t
//
//===---------------------------------------------------------------------------
