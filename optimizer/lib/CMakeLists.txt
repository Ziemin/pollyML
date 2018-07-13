add_library(PollyMLOptimizer STATIC
  exchange/json_exchange.cpp)

set_target_properties(PollyMLOptimizer PROPERTIES FOLDER "PollyMLOptimizer")

add_dependencies(PollyMLOptimizer Polly)

target_link_libraries(PollyMLOptimizer
    # ${Boost_LIBRARIES}
    Polly
  )

install(TARGETS PollyMLOptimizer
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin)
