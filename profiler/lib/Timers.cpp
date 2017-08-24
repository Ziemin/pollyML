#include <time.h>
#include <unistd.h>
#include <nlohmann/json.hpp>
#include <boost/chrono.hpp>
#include <boost/hana/tuple.hpp>
#include <boost/hana/transform.hpp>
#include <boost/hana/zip_with.hpp>
#include <boost/hana/minus.hpp>

#include "scop_profiler/Timers.h"
#include "scop_profiler/Papi.h"

namespace chr = boost::chrono;
namespace hana = boost::hana;
using json = nlohmann::json;

// ---- RDTSCTimer ------------------------------------------------------------
std::string_view RDTSCTimer::name = "RDTSC";

auto RDTSCTimer::print(std::ostream& out, uint64_t dur) const -> void {
  out << dur;
}

auto RDTSCTimer::to_json(json& data, uint64_t dur) const -> void {
  data = dur;
}


// ---- ClockTimer ------------------------------------------------------------
auto operator-(ClockTimer::timepoint_t end_time,
               ClockTimer::timepoint_t beg_time) -> ClockTimer::duration_t {

  timespec tp;
  tp.tv_sec = end_time.tv_sec - beg_time.tv_sec;
  tp.tv_nsec = end_time.tv_nsec - beg_time.tv_nsec;
  if (tp.tv_nsec < 0) {
    tp.tv_sec--;
    tp.tv_nsec += 1E9L;
  }

  return tp;
}
std::string_view ClockTimer::name = "Clock";

auto ClockTimer::print(std::ostream& out, const timespec& tm) const -> void {
  out << "Seconds: " << tm.tv_sec << " Nsecs: " << tm.tv_nsec;
}

auto ClockTimer::to_json(json& data, const timespec& tm) const -> void {
  data = int64_t(1E9L) * tm.tv_sec + tm.tv_nsec;
}


// ---- BoostUserTimer --------------------------------------------------------
std::string_view BoostUserTimer::name = "BoostUserCPU";

auto BoostUserTimer::print(std::ostream& out, const BoostUserTimer::duration_t& dur) const -> void {
  chr::seconds sdur = chr::duration_cast<chr::seconds>(dur);
  int64_t seconds = sdur.count();
  int64_t nseconds = dur.count() - chr::duration_cast<chr::nanoseconds>(sdur).count();
  out << "nsec total " << (int64_t)dur.count() << "; " << seconds << " sec " << nseconds << " nsec";
}

auto BoostUserTimer::to_json(json& data, const duration_t& dur) const -> void {
  data = (int64_t) dur.count();
}


// ---- PAPITimer -------------------------------------------------------------
std::string_view PAPITimer::name = "PAPICounters";

PAPITimer::PAPITimer(papi::EventSet_t event_set)
  : event_set(event_set)
  , num_events(PAPI_num_events(event_set))
  { 
    PAPI_list_events(event_set, event_codes, &num_events);
  }

auto PAPITimer::now() -> PAPITimer::timepoint_t {
  PAPITimer::timepoint_t counters(num_events, 0);
  if (PAPI_read(event_set, counters.data()) != PAPI_OK) {
    std::cerr << "Could not accumulate events!\n";
    exit(1);
  }
  return counters;
}

auto operator-(PAPITimer::timepoint_t end_time,
               PAPITimer::timepoint_t beg_time) -> PAPITimer::duration_t {
  auto difference = end_time;
  for (size_t i = 0; i < beg_time.size(); i++) {
    difference[i] -= beg_time[i];
  }
  return difference;
}

auto PAPITimer::print(std::ostream& out, const PAPITimer::duration_t& dur) const -> void {

  char event_code_str[PAPI_MAX_STR_LEN];

  out << '\n';
  for (int i = 0; i < num_events; i++) {
    PAPI_event_code_to_name(event_codes[i], event_code_str);
    out << '\t' << event_code_str << ": " << dur[i] << '\n';
  }
}

auto PAPITimer::to_json(json& data, const duration_t& dur) const -> void {

  char event_code_str[PAPI_MAX_STR_LEN];

  for (int i = 0; i < num_events; i++) {
    PAPI_event_code_to_name(event_codes[i], event_code_str);
    data[event_code_str] = dur[i];
  }
}
