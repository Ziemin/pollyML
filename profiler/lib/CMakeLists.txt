add_library(ScopProfiler
  profiling.cpp
  Timers.cpp)

target_link_libraries(ScopProfiler
    ${Boost_LIBRARIES}
  )
