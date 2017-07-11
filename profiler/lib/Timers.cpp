#include <time.h>
#include <unistd.h>
#include <boost/chrono.hpp>

#include "scop_profiler/Timers.h"

namespace chr = boost::chrono;

// ---- RDTSCTimer ------------------------------------------------------------
std::string_view RDTSCTimer::name = "RDTSC";


// ---- ClockTimer ------------------------------------------------------------
auto operator-(ClockTimer::timepoint_t end_time,
               ClockTimer::timepoint_t beg_time) -> ClockTimer::duration_t {

  timespec tp;
  tp.tv_sec = end_time.tv_sec - beg_time.tv_sec;
  tp.tv_nsec = end_time.tv_nsec - beg_time.tv_nsec;
  if (tp.tv_nsec < 0) {
    tp.tv_sec--;
    tp.tv_nsec += 10e9L;
  }

  return tp;
}
std::string_view ClockTimer::name = "Clock";

auto operator<<(std::ostream& out, timespec tm) -> std::ostream& {
  out << "Seconds: " << tm.tv_sec << "Nsecs: " << tm.tv_nsec;
  return out;
}


// ---- BoostUserTimer --------------------------------------------------------
std::string_view BoostUserTimer::name = "BoostUserCPU";

auto operator<<(std::ostream& out, const BoostUserTimer::duration_t& dur) -> std::ostream& {
  chr::seconds sdur = chr::duration_cast<chr::seconds>(dur);
  int64_t seconds = sdur.count();
  int64_t nseconds = dur.count() - chr::duration_cast<chr::nanoseconds>(sdur).count();
  out << "nsec total " << (int64_t)dur.count() << "; " << seconds << " sec " << nseconds << " nsec";
  return out;
}


// ---- PAPITimer -------------------------------------------------------------
