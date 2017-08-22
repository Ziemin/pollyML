
#add_library(PollyML
  #ScopProfiling.cpp
  #ProfilingInitializer.cpp
  #RegisterPasses.cpp
  #ProfilingCodegen.cpp)

##target_link_libraries(PollyML
    ##LLVMSupport
    ##LLVMCore
    ##Polly)

#add_dependencies(PollyML Polly)

#add_library(LLVMPollyML MODULE
  #PollyML.cpp)

#target_link_libraries(LLVMPollyML
    #PollyML)

#link_directories(
  #${LLVM_LIBRARY_DIR}
#)

#set_target_properties(LLVMPollyML
  #PROPERTIES
  #PREFIX "")

link_directories(
  ${LLVM_LIBRARY_DIR}
)

add_library(PollyMLCore OBJECT
  ScopProfiling.cpp
  ProfilingInitializer.cpp
  RegisterPasses.cpp
  ProfilingCodegen.cpp)

add_dependencies(PollyMLCore Polly)

add_polly_library(PollyML $<TARGET_OBJECTS:PollyMLCore>)
target_link_libraries(PollyML
    LLVMSupport
    LLVMCore
    Polly)

add_polly_loadable_module(LLVMPollyML
  PollyML.cpp
  $<TARGET_OBJECTS:PollyMLCore>
)

set_target_properties(LLVMPollyML
  PROPERTIES
  PREFIX "")
