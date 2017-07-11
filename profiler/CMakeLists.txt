project(ScopProfiler)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions -fno-rtti -std=c++1z")

find_package(PAPI REQUIRED)
include_directories(${PAPI_INCLUDE_DIRS})

find_package(Boost REQUIRED COMPONENTS chrono)
include_directories(${Boost_INCLUDE_DIRS})

include_directories(
  BEFORE
  ${CMAKE_CURRENT_SOURCE_DIR}/include/
  )

add_subdirectory(lib)
add_subdirectory(unittests)
