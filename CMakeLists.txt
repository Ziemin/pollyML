message("============== Polly-ML project =============")

if (NOT DEFINED LLVM_MAIN_SRC_DIR)
  message(FATAL_ERROR "PollyML is not a part of LLVM project")
endif()

# Find the source directory of Polly
if (NOT POLLY_SOURCE_DIR)
  get_filename_component(PARENT_DIR ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)
  set(POLLY_SOURCE_DIR ${PARENT_DIR}/polly)
endif ()
if (NOT EXISTS ${POLLY_SOURCE_DIR})
  message(FATAL_ERROR "There is no Polly directory in the project")
endif()
# Find the binary directory of Polly
if (NOT POLLY_BINARY_DIR)
  get_filename_component(PARENT_DIR ${CMAKE_CURRENT_BINARY_DIR} DIRECTORY)
  set(POLLY_BINARY_DIR ${PARENT_DIR}/polly)
endif ()

set(POLLY_INCLUDE_DIRS
  ${POLLY_SOURCE_DIR}/include
  ${POLLY_BINARY_DIR}/include)

set(ISL_INCLUDE_DIRS
  ${POLLY_SOURCE_DIR}/lib/External/isl/include
  ${POLLY_BINARY_DIR}/lib/External/isl/include)


# Add appropriate flags for GCC (Copied from Polly)
if (CMAKE_COMPILER_IS_GNUCXX)
  # FIXME: Turn off exceptions, RTTI:
  # -fno-exceptions -fno-rtti
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-common -Woverloaded-virtual -Wno-long-long -Wall -W -Wno-unused-parameter -Wwrite-strings -fno-exceptions -fno-rtti")
elseif (MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHs-c-")
  add_definitions("-D_HAS_EXCEPTIONS=0")
else ()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions -fno-rtti")
endif ()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z")

# Add include headers
include_directories(
  BEFORE
  ${CMAKE_CURRENT_SOURCE_DIR}/include/
  ${POLLY_INCLUDE_DIRS}
  ${ISL_INCLUDE_DIRS})

# Add path for custom modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include("pollyML_macros")

add_subdirectory(lib)
add_subdirectory(test)
add_subdirectory(unittests)
add_subdirectory(profiler)
add_subdirectory(optimizer)

# Add target to check formatting of pollyML files (Copied from Polly)
file(GLOB_RECURSE files *.h lib/*.cpp lib/*.c tools/*.cpp tools/*.c tools/*.h unittests/*.cpp)

set(check_format_depends)
set(update_format_depends)
set(i 0)
foreach (file IN LISTS files)
  add_custom_command(OUTPUT pollyML-check-format${i}
    COMMAND clang-format -sort-includes -style=llvm ${file} | diff -u ${file} -
    VERBATIM
    COMMENT "Checking format of ${file}..."
  )
  list(APPEND check_format_depends "pollyML-check-format${i}")

  add_custom_command(OUTPUT pollyML-update-format${i}
    COMMAND clang-format -sort-includes -i -style=llvm ${file}
    VERBATIM
    COMMENT "Updating format of ${file}..."
  )
  list(APPEND update_format_depends "pollyML-update-format${i}")

  math(EXPR i ${i}+1)
endforeach ()

add_custom_target(pollyML-check-format DEPENDS ${check_format_depends})
set_target_properties(pollyML-check-format PROPERTIES FOLDER "PollyML")

add_custom_target(pollyML-update-format DEPENDS ${update_format_depends})
set_target_properties(pollyML-update-format PROPERTIES FOLDER "PollyML")

#foreach (_variableName ${_variableNames})
    #message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()
