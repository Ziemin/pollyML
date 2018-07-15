project(PollyMLOptimizer)

string(REPLACE "-fno-rtti" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
string(REPLACE "-fno-exceptions" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DENABLE_DEBUG_PRINT")
add_definitions(-DJSON_NOEXCEPTION)

find_package(FMT REQUIRED)
include_directories(fmt::fmt)
link_directories(fmt::fmt)

# check_include_file("range/v3/all.hpp" HAVE_RANGE_V3)
# if(NOT HAVE_RANGE_V3)
#   message(FATAL_ERROR "range-v3 not found")
# endif()

# set(Boost_USE_STATIC_LIBS   ON)
# find_package(Boost REQUIRED COMPONENTS chrono system)
# include_directories(${Boost_INCLUDE_DIRS})

include_directories(
  BEFORE
  ${CMAKE_CURRENT_SOURCE_DIR}/include/
  )
# link_directories(${Boost_LIBRARY_DIRS})

add_subdirectory(lib)
add_subdirectory(unittests)
