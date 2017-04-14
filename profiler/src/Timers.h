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
//     duration(timepoint_t first, timepoint_t second) -> duration_t;
//  }
//
//===---------------------------------------------------------------------------
