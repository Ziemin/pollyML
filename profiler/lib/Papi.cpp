#include "scop_profiler/Papi.h"

#include <assert.h>
#include <iostream>
#include <papi.h>

namespace papi {

auto init_papi(int events_num, const char **event_code_names) -> EventSet_t {

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
  for (int i = 0; i < events_num; i++) {
    if (PAPI_event_name_to_code(const_cast<char *>(event_code_names[i]),
                                &event_code) != PAPI_OK) {
      std::cerr << "Invalid PAPI event name: " << event_code_names[i] << "!\n";
      exit(1);
    }
    if (PAPI_add_event(event_set, event_code) != PAPI_OK) {
      std::cerr << "Counter for " << event_code_names[i]
                << " is not available!\n";
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
