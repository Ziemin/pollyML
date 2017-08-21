#include "gtest/gtest.h"
#include <boost/hana/tuple.hpp>
#include <chrono>
#include <iostream>
#include <nlohmann/json.hpp>
#include <vector>
#include <thread>

#include "scop_profiler/Statistics.h"
#include "scop_profiler/Timers.h"

using namespace std::chrono_literals;

namespace {

TEST(ScopStatistics, FewTimers) {

  using TimeStatistics =
      Statistics<RDTSCTimer, ClockTimer, BoostUserTimer, PAPITimer>;

  const std::vector<std::string> papiEventNames{
    "PAPI_TOT_CYC", "PAPI_TOT_INS", "PAPI_L1_DCM", "PAPI_BR_MSP"};

  papi::EventSet_t event_set = papi::init_papi(papiEventNames);

  TimeStatistics stats{RDTSCTimer(), ClockTimer(), BoostUserTimer(),
                       PAPITimer(event_set)};

  stats.startProfiling("FirstRegion", {{"Ala", 1}, {"Ola", 2}, {"Ula", 3}});
  stats.stopProfiling("FirstRegion");

  stats.startProfiling("SecondRegion",
                       {{"P1", 10000}, {"P2", 1231}, {"P3", -10}});
  stats.stopProfiling("SecondRegion");

  stats.startProfiling("FirstRegion", {{"Ala", 10}, {"Ola", 20}, {"Ula", 30}});
  stats.stopProfiling("FirstRegion");

  stats.startProfiling("ThirdRegion", {});
  stats.stopProfiling("ThirdRegion");

  auto serialized = stats.serializeToJson();
  std::cout << std::setw(4) << serialized << std::endl;

  ASSERT_EQ(serialized.size(), 3UL);
  ASSERT_TRUE(serialized.find("FirstRegion") != serialized.end());
  ASSERT_TRUE(serialized.find("SecondRegion") != serialized.end());
  ASSERT_TRUE(serialized.find("ThirdRegion") != serialized.end());
  ASSERT_EQ(serialized["FirstRegion"]["parameter_names"].size(), 3UL);
  ASSERT_EQ(serialized["FirstRegion"]["results"].size(), 2UL);
  ASSERT_EQ(serialized["FirstRegion"]["results"][0]["parameters"].size(), 3UL);
  ASSERT_EQ(serialized["FirstRegion"]["results"][0]["timers"].size(), 4UL);
}

} // anonymous namespace
