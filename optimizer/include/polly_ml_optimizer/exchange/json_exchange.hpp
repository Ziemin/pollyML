//===------ json_exchange.h ------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// T
//
//===----------------------------------------------------------------------===//

#pragma once

#include "polly_ml_optimizer/scop.hpp"

#include <nlohmann/json.hpp>
#include <string>
#include <variant>

namespace pollyML {

  std::variant<Scop, std::string> from_scopson(const nlohmann::json &scopson);

} // namespace pollyML
