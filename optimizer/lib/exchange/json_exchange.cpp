#include <polly_ml_optimizer/exchange/json_exchange.hpp>
#include <polly_ml_optimizer/isl_tools.hpp>

#include "isl/ctx.h"
#include "isl/isl-noexceptions.h"

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <nlohmann/json.hpp>
#include <range/v3/all.hpp>
#include <string>
#include <vector>

using namespace pollyML;
using json = nlohmann::json;
using namespace fmt::literals;

namespace {

std::variant<MemRef, std::string> parse_mem_ref(isl_ctx *isl_ctxt,
                                                const json &mem_ref_json) {
  std::string name;
  if (auto it = mem_ref_json.find("name"); it != mem_ref_json.end()) {
    name = it->get<std::string>();
  } else {
    return "Scopson does not have a field 'name'";
  }

  MemType mem_type;
  if (auto it = mem_ref_json.find("type"); it != mem_ref_json.end()) {
    auto maybe_mem_type = string_to_MemType(it->get<std::string>());
    if (maybe_mem_type)
      mem_type = *maybe_mem_type;
    else
      return "Memory type could not be parsed";
  } else {
    return "Scopson does not have a field 'name'";
  }

  Shape shape;
  if (auto it = mem_ref_json.find("sizes"); it != mem_ref_json.end()) {
    for (const auto &s_repr : mem_ref_json["sizes"]) {
      if (auto size_str = s_repr.get<std::string>(); size_str == "*")
        shape.push_back({});
      else
        shape.push_back(std::stoi(size_str));
    }
  }

  return MemRef(isl_ctxt, std::move(name), mem_type, std::move(shape));
}

std::variant<MemoryAccess, std::string> parse_access(isl_ctx *isl_ctxt,
                                                     const json &access_json) {
  AccessType acc_type;
  if (auto it = access_json.find("kind"); it != access_json.end()) {
    auto maybe_acc_type = string_to_AccessType(it->get<std::string>());
    if (maybe_acc_type)
      acc_type = *maybe_acc_type;
    else
      return "Access type could not be parsed";
  } else {
    return "Scopson access does not have a field 'name'";
  }

  isl::map access_relation;
  if (auto it = access_json.find("relation"); it != access_json.end()) {
    access_relation = isl::map(isl_ctxt, it->get<std::string>());
    if (!access_relation)
      return "Access relation string could not be parsed";
  } else {
    return "Scopson statement does not have a field 'name'";
  }

  return MemoryAccess(acc_type, access_relation);
}

std::variant<Statement, std::string>
parse_statement(isl_ctx *isl_ctxt, const json &statement_json) {

  std::string name;
  if (auto it = statement_json.find("name"); it != statement_json.end()) {
    name = it->get<std::string>();
  } else {
    return "Scopson statement does not have a field 'name'";
  }

  isl::set domain;
  if (auto it = statement_json.find("domain"); it != statement_json.end()) {
    domain = isl::set(isl_ctxt, it->get<std::string>());
    if (!domain.get()) {
      return "Domain string for statement {} could not be parsed"_format(name);
    }
  } else {
    return "Scopson statement does not have a field 'domain'";
  }

  isl::map schedule;
  if (auto it = statement_json.find("schedule"); it != statement_json.end()) {
    schedule = isl::map(isl_ctxt, it->get<std::string>());
    if (!schedule) {
      return "Schedule string for statement {} could not be parsed"_format(
          name);
    }
  } else {
    return "Scopson statement does not have a field 'schedule'";
  }

  std::vector<MemoryAccess> accesses;
  if (auto it = statement_json.find("accesses"); it != statement_json.end()) {
    for (const auto &acc_json : *it) {
      auto acc_result = parse_access(isl_ctxt, acc_json);
      if (std::holds_alternative<std::string>(acc_result))
        return std::get<std::string>(acc_result);
      accesses.push_back(std::move(std::get<MemoryAccess>(acc_result)));
    }
  } else {
    return "Scopson statement does not have a field 'accesses'";
  }

  return Statement(domain, schedule, std::move(accesses));
}

std::variant<Dependences, std::string>
parse_dependences(isl_ctx *isl_ctxt, const json &deps_json) {
  isl::union_map RAW, WAR, WAW, RED, TC_RED;

  std::string_view fields[] = {"RAW", "WAR", "WAW", "RED", "TC_RED"};
  isl::union_map *maps[] = {&RAW, &WAR, &WAW, &RED, &TC_RED};
  auto range = ranges::view::zip(fields, maps);
  for (auto [field_name, map_ptr]: range) {
    if (auto it = deps_json.find(field_name.data()); it != deps_json.end()) {
      *map_ptr = isl::union_map(isl_ctxt, it->get<std::string>());
      if (!map_ptr->get()) {
        return "{} dependences could not be parsed"_format(field_name);
      }
    } else {
      return "Scopson statement does not have a field 'domain'";
    }
  }
  return Dependences(RAW, WAR, WAW, RED, TC_RED);
}

} // anonymous namespace

std::variant<Scop, std::string>
pollyML::from_scopson(const nlohmann::json &scopson) {
  IslCtxUPtr ctxt(isl_ctx_alloc());

  std::string scop_name;
  if (auto it = scopson.find("name"); it != scopson.end()) {
    scop_name = it->get<std::string>();
  } else {
    return "Scopson does not have a field 'name'";
  }

  isl::set scop_context;
  if (auto it = scopson.find("context"); it != scopson.end()) {
    std::string context_str = it->get<std::string>();
    scop_context = isl::set(ctxt.get(), context_str);
    if (scop_context.get() == nullptr)
      return "Could not parse context string";
  } else {
    return "Scopson does not have a field 'context'";
  }

  std::vector<MemRef> memory_refs;

  if (auto it = scopson.find("arrays"); it != scopson.end()) {
    for (const json &mem_ref_json : *it) {
      auto mem_ref_res = parse_mem_ref(ctxt.get(), mem_ref_json);
      if (std::holds_alternative<std::string>(mem_ref_res))
        return std::get<std::string>(mem_ref_res);
      memory_refs.push_back(std::move(std::get<MemRef>(mem_ref_res)));
    }
  } else {
    return "Scopson does not have a field 'arrays'";
  }

  std::vector<Statement> statements;
  if (auto it = scopson.find("statements"); it != scopson.end()) {
    for (const json &statement_json : *it) {
      auto statement_res = parse_statement(ctxt.get(), statement_json);
      if (std::holds_alternative<std::string>(statement_res))
        return std::get<std::string>(statement_res);
      statements.push_back(std::move(std::get<Statement>(statement_res)));
    }
  } else {
    return "Scopson does not have a field 'statements";
  }

  if (auto it = scopson.find("dependences"); it != scopson.end()) {
    auto deps_res = parse_dependences(ctxt.get(), *it);
    if (std::holds_alternative<std::string>(deps_res))
      return std::get<std::string>(deps_res);

    return Scop(std::move(ctxt), std::move(scop_name), scop_context,
                std::move(memory_refs), std::move(statements),
                std::move(std::get<Dependences>(deps_res)));
  } else {
    return "Scopson does not have a field 'statements";
  }

  return "err";
}
