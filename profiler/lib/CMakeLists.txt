add_library(ScopProfiler SHARED
  profiling.cpp
  Timers.cpp
  Papi.cpp
  Utils.cpp)

target_link_libraries(ScopProfiler
    ${Boost_LIBRARIES}
    ${PAPI_LIBRARIES}
    LLVMCore
  )
install(TARGETS ScopProfiler
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin)
