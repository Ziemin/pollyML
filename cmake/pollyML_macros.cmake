include(CMakeParseArguments)

macro(add_pollyML_library name)
  cmake_parse_arguments(ARG "" "" "" ${ARGN})
  set(srcs ${ARG_UNPARSED_ARGUMENTS})
  if (MODULE)
    set(libkind MODULE)
  elseif (SHARED_LIBRARY)
    set(libkind SHARED)
  else()
    set(libkind)
  endif()
  add_library( ${name} ${libkind} ${srcs} )
  set_target_properties(${name} PROPERTIES FOLDER "PollyML")

  if( LLVM_COMMON_DEPENDS )
    add_dependencies( ${name} ${LLVM_COMMON_DEPENDS} )
  endif( LLVM_COMMON_DEPENDS )
  if( LLVM_USED_LIBS )
    foreach(lib ${LLVM_USED_LIBS})
      target_link_libraries( ${name} ${lib} )
    endforeach(lib)
  endif( LLVM_USED_LIBS )

  if(POLLY_ML_LINK_LIBS)
    foreach(lib ${POLLY_ML_LINK_LIBS})
      target_link_libraries(${name} ${lib})
    endforeach(lib)
  endif(POLLY_ML_LINK_LIBS)

  if( LLVM_LINK_COMPONENTS )
    llvm_config(${name} ${LLVM_LINK_COMPONENTS})
  endif( LLVM_LINK_COMPONENTS )
  if (NOT LLVM_INSTALL_TOOLCHAIN_ONLY OR ${name} STREQUAL "LLVMPollyML")
    install(TARGETS ${name}
      EXPORT LLVMExports
      LIBRARY DESTINATION lib${LLVM_LIBDIR_SUFFIX}
      ARCHIVE DESTINATION lib${LLVM_LIBDIR_SUFFIX})
  endif()
  set_property(GLOBAL APPEND PROPERTY LLVM_EXPORTS ${name})
endmacro(add_pollyML_library)

macro(add_pollyML_loadable_module name)
  set(srcs ${ARGN})
  # klduge: pass different values for MODULE with multiple targets in same dir
  # this allows building shared-lib and module in same dir
  # there must be a cleaner way to achieve this....
  if (MODULE)
  else()
    set(GLOBAL_NOT_MODULE TRUE)
  endif()
  set(MODULE TRUE)
  add_pollyML_library(${name} ${srcs})
  set_target_properties(${name} PROPERTIES FOLDER "PollyML")
  if (GLOBAL_NOT_MODULE)
    unset (MODULE)
  endif()
endmacro(add_pollyML_loadable_module)
