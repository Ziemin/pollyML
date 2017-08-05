project(ScopProfiler)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions -fno-rtti -std=c++1z")
add_definitions(-DJSON_NOEXCEPTION)

find_package(PAPI REQUIRED)
include_directories(${PAPI_INCLUDE_DIRS})

set(Boost_USE_STATIC_LIBS   ON)
find_package(Boost REQUIRED COMPONENTS chrono system)
include_directories(${Boost_INCLUDE_DIRS})

include_directories(
  BEFORE
  ${CMAKE_CURRENT_SOURCE_DIR}/include/
  )
link_directories(${Boost_LIBRARY_DIRS})

add_subdirectory(lib)
add_subdirectory(unittests)
