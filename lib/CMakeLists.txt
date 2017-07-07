
add_library(PollyML
  ScopProfiling.cpp
  RegisterPasses.cpp
  ProfilingCodegen.cpp)

#target_link_libraries(PollyML
    #LLVMSupport
    #LLVMCore
    #Polly)

add_dependencies(PollyML Polly)

add_library(LLVMPollyML MODULE
  PollyML.cpp)

target_link_libraries(LLVMPollyML
    PollyML)

link_directories(
  ${LLVM_LIBRARY_DIR}
)

set_target_properties(LLVMPollyML
  PROPERTIES
  PREFIX "")
