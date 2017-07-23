add_library(ScopProfiler SHARED
  profiling.cpp
  Timers.cpp
  Papi.cpp)

target_link_libraries(ScopProfiler
    ${Boost_LIBRARIES}
    ${PAPI_LIBRARIES}
    LLVMCore
  )
