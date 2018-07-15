//===- optimizer/scop.h -----------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Store the polyhedral model representation of a static control flow region,
// also called SCoP (Static Control Part).
//
//===----------------------------------------------------------------------===//

#pragma once

#include "polly_ml_optimizer/utils.hpp"
#include "polly_ml_optimizer/isl_tools.hpp"

#include "isl/isl-noexceptions.h"

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>
#include <ostream>
#include <cassert>

namespace pollyML {

enum class MemType { FLOAT, DOUBLE, INT, LONG };
std::optional<MemType> string_to_MemType(std::string_view mem_type_str);
std::ostream& operator<<(std::ostream& out, MemType mem_type);

enum class AccessType { READ, WRITE };
std::optional<AccessType> string_to_AccessType(std::string_view access_type_str);
std::ostream& operator<<(std::ostream& out, AccessType access_type);

using Shape = std::vector<std::optional<int64_t>>;

struct MemRef : private NonCopyable<MemRef> {

  isl::id id;
  std::string name;
  MemType mem_type;
  std::vector<std::optional<int64_t>> shape;

  MemRef(isl::ctx ctxt, std::string name, MemType mem_type, Shape shape);

  MemRef(MemRef &&other) = default;
  MemRef &operator=(MemRef &&other) = default;

  int dim() const { return shape.size(); }
  bool is_array() const { return shape.empty(); }

};

std::ostream& operator<<(std::ostream& out, const MemRef& mem_ref);


struct MemoryAccess : private NonCopyable<MemoryAccess> {

  AccessType acc_type;
  isl::map acc_relation;

  MemoryAccess(AccessType acc_type, isl::map acc_rel);

  MemoryAccess(MemoryAccess &&other) = default;
  MemoryAccess &operator=(MemoryAccess &&other) = default;

};

std::ostream& operator<<(std::ostream& out, const MemoryAccess &mem_acc);

struct Statement : private NonCopyable<Statement> {

  isl::id id;
  std::string name;
  isl::set domain;
  isl::map schedule;
  std::vector<MemoryAccess> mem_accs;

  Statement(isl::set domain, isl::map schedule,
            std::vector<MemoryAccess> accesses);

  Statement(Statement &&other) = default;
  Statement &operator=(Statement &&other) = default;

};

std::ostream& operator<<(std::ostream& out, const Statement &stmt);

struct Dependences : private NonCopyable<Dependences> {
  isl::union_map RAW, WAR, WAW, RED, TC_RED;

  Dependences(isl::union_map RAW, isl::union_map WAR, isl::union_map WAW,
              isl::union_map RED, isl::union_map TC_RED);

  Dependences(Dependences &&other) = default;
  Dependences &operator=(Dependences &&other) = default;

};

std::ostream& operator<<(std::ostream& out, const Dependences &deps);

struct Scop : private NonCopyable<Scop> {

  IslCtxUPtr isl_ctxt;
  std::string name;
  isl::set context;
  std::vector<MemRef> memory_refs;
  std::vector<Statement> stmts;
  Dependences deps;

  Scop(IslCtxUPtr isl_ctxt, std::string name, isl::set context,
       std::vector<MemRef> memory_refs, std::vector<Statement> stmts,
       Dependences deps);

  Scop(Scop &&other) = default;
  Scop &operator=(Scop &&other) = default;

};
std::ostream& operator<<(std::ostream& out, const Scop &scop);

} // namespace pollyML
