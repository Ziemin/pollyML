add_library(ScopProfiler SHARED
  profiling.cpp
  Timers.cpp)

target_link_libraries(ScopProfiler
    ${Boost_LIBRARIES}
    LLVMCore
  )
