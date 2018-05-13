add_custom_target(ScopProfilerUnitTests)
set_target_properties(ScopProfilerUnitTests PROPERTIES FOLDER "PollyML - ScopProfiler Tests")

function(add_scop_profiler_unittest test_name)
  add_unittest(ScopProfilerUnitTests ${test_name} ${ARGN})
  target_link_libraries(${test_name} PRIVATE
    ScopProfiler
  )
endfunction()

add_scop_profiler_unittest(StatisticsTest
  StatisticsTest.cpp
  )
