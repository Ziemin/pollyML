#include "gtest/gtest.h"
#include <boost/hana/tuple.hpp>
#include <thread>
#include <chrono>

#include "scop_profiler/Statistics.h"
#include "scop_profiler/Timers.h"

using namespace std::chrono_literals;

namespace {

TEST(ScopStatistics, FewTimers) {

  using TimeStatistics = Statistics<RDTSCTimer, ClockTimer, BoostUserTimer>;

  TimeStatistics stats;

  params_t params = { {"Ala", 1}, {"Ola", 2}, {"Ula", 3} };

  stats.startProfiling("FirstRegion", std::move(params));
  stats.stopProfiling("FirstRegion");
}

} // anonymous namespace
