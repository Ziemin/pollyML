#include "polly_ml_optimizer/scop.hpp"

#include "isl/isl-noexceptions.h"

#include <cassert>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <iomanip>
#include <range/v3/all.hpp>

using namespace pollyML;
using namespace fmt::literals;

std::optional<MemType>
pollyML::string_to_MemType(std::string_view mem_type_str) {
  if (mem_type_str == "float")
    return MemType::FLOAT;
  else if (mem_type_str == "double")
    return MemType::DOUBLE;
  else if (mem_type_str == "int")
    return MemType::INT;
  else if (mem_type_str == "long")
    return MemType::LONG;
  else
    return {};
}

std::ostream &pollyML::operator<<(std::ostream &out, MemType mem_type) {
  switch (mem_type) {
  case MemType::FLOAT:
    out << "float";
    break;
  case MemType::DOUBLE:
    out << "double";
    break;
  case MemType::INT:
    out << "int";
    break;
  case MemType::LONG:
    out << "long";
    break;
  }
  return out;
}

std::optional<AccessType>
pollyML::string_to_AccessType(std::string_view access_type_str) {
  if (access_type_str == "read")
    return AccessType::READ;
  else if (access_type_str == "write")
    return AccessType::WRITE;
  else
    return {};
}

std::ostream &pollyML::operator<<(std::ostream &out, AccessType access_type) {
  switch (access_type) {
  case AccessType::READ:
    out << "read";
    break;
  case AccessType::WRITE:
    out << "write";
    break;
  }
  return out;
}

MemRef::MemRef(isl::ctx ctxt, std::string name, MemType mem_type, Shape shape)
    : name(std::move(name)), mem_type(mem_type), shape(std::move(shape)) {
  id = isl::id::alloc(ctxt, this->name, nullptr);
}

std::ostream &pollyML::operator<<(std::ostream &out, const MemRef &mem_ref) {
  out << "\tname: {0}\n"_format(mem_ref.name);
  out << "\ttype: {0}\n"_format(mem_ref.mem_type);
  auto rng = mem_ref.shape | ranges::view::transform([](auto el) -> std::string {
               if (el)
                 return fmt::to_string(*el);
               else
                 return "*";
             });
  out << "\tshape: {0}"_format(rng);
  return out;
}

MemoryAccess::MemoryAccess(AccessType acc_type, isl::map acc_rel)
    : acc_type(acc_type), acc_relation(acc_rel) {}

std::ostream &pollyML::operator<<(std::ostream &out,
                                  const MemoryAccess &mem_acc) {
  out << "\t\ttype: {0}\n"_format(mem_acc.acc_type);
  out << "\t\trelation: {0}\n"_format(mem_acc.acc_relation.to_str());
  return out;
}

Statement::Statement(isl::set domain, isl::map schedule,
                     std::vector<MemoryAccess> accesses)
    : id(domain.get_tuple_id()), domain(domain), schedule(schedule) {
  name = id.get_name();

  assert(id.get() == schedule.get_tuple_id(isl::dim::in).get() &&
         "Domain and schedule statement tuple ids should be the same");

  for (auto &acc : accesses) {
    assert(id.get() == acc.acc_relation.get_tuple_id(isl::dim::in).get() &&
           "Statement and access should have the tuple id");
  }

  mem_accs = std::move(accesses);
}

std::ostream &pollyML::operator<<(std::ostream &out, const Statement &stmt) {
  out << "\tname: {0}\n"_format(stmt.name);
  out << "\tdomain: {0}\n"_format(stmt.domain.to_str());
  out << "\tschedule: {0}\n"_format(stmt.schedule.to_str());
  out << "\tmemory accesses:\n";
  for (const auto &acc : stmt.mem_accs)
    out << acc << '\n';

  return out;
}

Dependences::Dependences(isl::union_map RAW, isl::union_map WAR,
                         isl::union_map WAW, isl::union_map RED,
                         isl::union_map TC_RED)
    : RAW(RAW), WAR(WAR), WAW(WAW), RED(RED), TC_RED(TC_RED) {}

std::ostream &pollyML::operator<<(std::ostream &out, const Dependences &deps) {
  out << "\tRAW: {}\n\n"_format(deps.RAW.to_str());
  out << "\tWAR: {}\n\n"_format(deps.WAR.to_str());
  out << "\tWAW: {}\n\n"_format(deps.WAW.to_str());
  out << "\tRED: {}\n\n"_format(deps.RED.to_str());
  out << "\tTC_RED: {}\n\n"_format(deps.TC_RED.to_str());
  return out;
}

Scop::Scop(IslCtxUPtr isl_ctxt, std::string name, isl::set context,
           std::vector<MemRef> memory_refs, std::vector<Statement> stmts,
           Dependences deps)
    : isl_ctxt(std::move(isl_ctxt)), name(std::move(name)), context(context),
      memory_refs(std::move(memory_refs)), stmts(std::move(stmts)),
      deps(std::move(deps)) {}

std::ostream &pollyML::operator<<(std::ostream &out, const Scop &scop) {
  out << "name: {}\n\n"_format(scop.name);
  out << "context: {}\n\n"_format(scop.context.to_str());

  out << "memory refs:\n";
  out << "\t-----------------------------\n";
  for (const auto &mem_ref: scop.memory_refs) {
    out << mem_ref << '\n';
    out << "\t-----------------------------\n";
  }

  out << "\nstatements:\n";
  out << "\t-----------------------------\n";
  for (const auto &stmt: scop.stmts) {
    out << stmt << '\n';
    out << "\t-----------------------------\n";
  }

  out << "\ndependences:\n";
  out << scop.deps << '\n';

  return out;
}
