link_directories(
  ${LLVM_LIBRARY_DIR}
)

add_library(PollyMLCore OBJECT
  ScopProfiling.cpp
  ProfilingInitializer.cpp
  RegisterPasses.cpp
  ProfilingCodegen.cpp
  ScopGraphInfo.cpp
  JsonExporter.cpp)

set_target_properties(PollyMLCore PROPERTIES FOLDER "PollyML")

add_dependencies(PollyMLCore Polly)

add_pollyML_library(PollyML $<TARGET_OBJECTS:PollyMLCore>)
target_link_libraries(PollyML
    LLVMSupport
    LLVMCore
    Polly)

add_pollyML_loadable_module(LLVMPollyML
  PollyML.cpp
  $<TARGET_OBJECTS:PollyMLCore>
)

set_target_properties(LLVMPollyML
  PROPERTIES
  PREFIX "")
