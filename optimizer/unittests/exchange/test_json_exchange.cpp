#include "polly_ml_optimizer/exchange/json_exchange.hpp"
#include "polly_ml_optimizer/scop.hpp"

#include "isl/id.h"

#include "gtest/gtest.h"
#include <fstream>
#include <iostream>
#include <variant>
#include <unordered_set>
#include <nlohmann/json.hpp>

#ifndef POLLY_ML_OPTIMIZER_TEST_DATA_DIR
#define POLLY_ML_OPTIMIZER_TEST_DATA_DIR "."
#endif

using json = nlohmann::json;

const std::string DATA_DIR = POLLY_ML_OPTIMIZER_TEST_DATA_DIR;

TEST(ScopsonImport, kernel_2mm) {
  std::string scopson_path =
      DATA_DIR + "/exchange/kernel_2mm___%.split---%45.scopson";
  std::ifstream scopson_file(scopson_path);
  ASSERT_TRUE(scopson_file);

  json kernel_json;
  scopson_file >> kernel_json;

  auto parse_result = pollyML::from_scopson(kernel_json);
  ASSERT_TRUE(std::holds_alternative<pollyML::Scop>(parse_result));

  pollyML::Scop scop = std::move(std::get<pollyML::Scop>(parse_result));

  ASSERT_EQ(scop.name, "%.split---%45");
  ASSERT_EQ(scop.memory_refs.size(), 7UL);
  ASSERT_EQ(scop.context.to_str(),  "[p_0, p_1, p_2, p_3] -> {  : -2147483648 <= p_0 <= 2147483647 and -2147483648 <= p_1 <= 2147483647 and -2147483648 <= p_2 <= 2147483647 and -2147483648 <= p_3 <= 2147483647 }");
  // std::cerr << scop;
  ASSERT_EQ(scop.deps.RAW.to_str(),  "[p_0, p_1, p_2, p_3] -> { Stmt16[i0, i1] -> Stmt18[i0, i1, o2] : 0 <= i0 < p_0 and 0 <= i1 < p_3 and 0 <= o2 < p_1; Stmt6[i0, i1, i2] -> Stmt18[i0, o1, i1] : 0 <= i0 < p_0 and 0 <= i1 < p_1 and 0 <= i2 < p_2 and 0 <= o1 < p_3; Stmt4[i0, i1] -> Stmt6[i0, i1, o2] : 0 <= i0 < p_0 and 0 <= i1 < p_1 and 0 <= o2 < p_2; Stmt4[i0, i1] -> Stmt18[i0, o1, i1] : p_2 <= 0 and 0 <= i0 < p_0 and 0 <= i1 < p_1 and 0 <= o1 < p_3 }");
  ASSERT_EQ(scop.deps.WAR.to_str(), "[p_0, p_1, p_2, p_3] -> { Stmt16[i0, i1] -> Stmt18[i0, i1, o2] : 0 <= i0 < p_0 and 0 <= i1 < p_3 and 0 <= o2 < p_1 }");
  ASSERT_EQ(scop.deps.WAW.to_str(), "[p_0, p_1, p_2, p_3] -> { Stmt16[i0, i1] -> Stmt18[i0, i1, o2] : 0 <= i0 < p_0 and 0 <= i1 < p_3 and 0 <= o2 < p_1; Stmt4[i0, i1] -> Stmt6[i0, i1, o2] : 0 <= i0 < p_0 and 0 <= i1 < p_1 and 0 <= o2 < p_2 }");
  ASSERT_EQ(scop.deps.RED.to_str(),  "[p_0, p_1, p_2, p_3] -> { Stmt18[i0, i1, i2] -> Stmt18[i0, i1, 1 + i2] : 0 <= i0 < p_0 and 0 <= i1 < p_3 and 0 <= i2 <= -2 + p_1; Stmt6[i0, i1, i2] -> Stmt6[i0, i1, 1 + i2] : 0 <= i0 < p_0 and 0 <= i1 < p_1 and 0 <= i2 <= -2 + p_2 }");
  ASSERT_EQ(scop.deps.TC_RED.to_str(),  "[p_0, p_1, p_2, p_3] -> { Stmt18[i0, i1, i2] -> Stmt18[i0, i1, o2] : 0 <= i0 < p_0 and 0 <= i1 < p_3 and ((i2 >= 0 and i2 < o2 < p_1) or (i2 < p_1 and 0 <= o2 < i2)); Stmt6[i0, i1, i2] -> Stmt6[i0, i1, o2] : 0 <= i0 < p_0 and 0 <= i1 < p_1 and ((i2 >= 0 and i2 < o2 < p_2) or (i2 < p_2 and 0 <= o2 < i2)) }");

  std::unordered_set<isl_id*> arrays_id_refs;
  for (const auto& mem_ref: scop.memory_refs)
    arrays_id_refs.insert(mem_ref.id.get());

  for (const auto& stmt: scop.stmts) {
    for (const auto& mem_acc: stmt.mem_accs) {
      isl::id mem_ref_id = mem_acc.acc_relation.get_tuple_id(isl::dim::out);
      ASSERT_TRUE(arrays_id_refs.find(mem_ref_id.get()) != end(arrays_id_refs));
    }
  }
}
