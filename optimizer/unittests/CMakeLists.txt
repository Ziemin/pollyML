add_custom_target(PollyMLOptimizerUnitTests)

set_target_properties(ScopProfilerUnitTests PROPERTIES FOLDER "PollyMLOptimizer")

add_definitions(-DPOLLY_ML_OPTIMIZER_TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")

function(add_polly_ml_optimizer_unittest test_name)
  add_unittest(PollyMLOptimizerUnitTests ${test_name} ${ARGN})
  target_link_libraries(${test_name} PRIVATE
    PollyMLOptimizer
  )
endfunction()


file(GLOB_RECURSE test_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} test_*.cpp)


foreach (file IN LISTS test_files)
  get_filename_component(test_name ${file} NAME_WE)
  add_polly_ml_optimizer_unittest(${test_name} ${file})
endforeach()
