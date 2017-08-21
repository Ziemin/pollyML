#include "scop_profiler/Papi.h"

#include <assert.h>
#include <iostream>
#include <vector>
#include <papi.h>

namespace papi {

auto init_papi(const std::vector<std::string>& papi_events) -> EventSet_t {

  if (PAPI_library_init(PAPI_VER_CURRENT) != PAPI_VER_CURRENT) {
    std::cerr << "PAPI library init error!\n";
    exit(1);
  }

  EventSet_t event_set = PAPI_NULL;
  int event_code;

  if (PAPI_create_eventset(&event_set) != PAPI_OK) {
    std::cerr << "Cannot create eventset!\n";
    exit(1);
  }

  // adding events to record
  for (const std::string& event_name: papi_events) {
    if (PAPI_event_name_to_code(const_cast<char *>(event_name.data()),
                                &event_code) != PAPI_OK) {
      std::cerr << "Invalid PAPI event name: " << event_name  << "!\n";
      exit(1);
    }
    if (PAPI_add_event(event_set, event_code) != PAPI_OK) {
      std::cerr << "Counter for " << event_name << " is not available!\n";
      exit(1);
    }
  }

  if (PAPI_start(event_set) != PAPI_OK) {
    std::cerr << "Cannot start counters!\n";
    exit(1);
  }

  return event_set;
}

} // namespace papi
