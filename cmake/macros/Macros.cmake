MACRO(SUBDIRLIST result curdir)
  FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
  SET(dirlist "")
  FOREACH(child ${children})
    IF(IS_DIRECTORY ${curdir}/${child})
        LIST(APPEND dirlist ${curdir}/${child})
    ENDIF()
  ENDFOREACH()
  SET(${result} ${dirlist})
ENDMACRO()

FUNCTION(CreateLibrary LibName LibSource Linkage Dependencis LibLinkFlags)

    link_directories(${PROJECTDEPS_PATH}/lib)

    add_library(${LibName} ${Linkage} ${LibSource} )

    IF(EAC)
        IF (${Linkage} STREQUAL "SHARED")
            SETUP_EDIT_AND_CONTINUE("${LibName}" 1)
        ELSE()
            SETUP_EDIT_AND_CONTINUE("${LibName}" 0)
        ENDIF()
    ENDIF(EAC)

    IF (${Linkage} STREQUAL "SHARED")
        target_link_libraries(${LibName}
          ${Dependencis}
        )

        # LIBRARY = dyld / so, RUNTIME = dll
        if( UNIX )
          install(TARGETS ${LibName} LIBRARY DESTINATION "${LIBS_DIR}")
        elseif( WIN32 )
          install(TARGETS ${LibName} RUNTIME DESTINATION "${LIBS_DIR}")
        endif(UNIX)

        message(${DllSTATUS} ${LibName})
    ELSE()
        message(${LibSTATUS} ${LibName})
    ENDIF(${Linkage} STREQUAL "SHARED")

ENDFUNCTION()

FUNCTION(CreateHotSwap LibName LibSource Linkage Dependencis LibLinkFlags)

    link_directories(${PROJECTDEPS_PATH}/lib)

    add_library(${LibName} ${Linkage} ${LibSource} )

    IF (${Linkage} STREQUAL "SHARED")
        target_link_libraries(${LibName} ${Dependencis} )

        link_directories(${PROJECTDEPS_PATH}/lib)

        if( WIN32 )
          IF(EAC)
            string(REPLACE "/ZI" "" DowngradedCompileFlags ${CMAKE_CXX_FLAGS_DEBUG})
            set_target_properties(${LibName} PROPERTIES COMPILE_FLAGS "${DowngradedCompileFlags}" )
          ENDIF(EAC)
          target_link_libraries(${LibName} "-DEBUG") # optimize for dynamic pdb
          #target_link_libraries(${LibName} "-INCREMENTAL:NO") # optimize for dynamic pdb
          target_link_libraries(${LibName} "-PDBALTPATH:%_PDB%")
          set_target_properties(${LibName} PROPERTIES PDB_NAME "$(TargetName)-$([System.DateTime]::Now.ToString(\"HH_mm_ss_fff\"))")
          add_custom_command(TARGET ${LibName} PRE_LINK COMMAND del $(OutDir)$(MSBuildProjectName)-*.pdb COMMENT "Delete pdb files")
        endif( WIN32 )

        # LIBRARY = dyld / so, RUNTIME = dll
        if( UNIX )
          install(TARGETS ${LibName} LIBRARY DESTINATION "${BIN_DIR}")
        elseif( WIN32 )
          install(TARGETS ${LibName} RUNTIME DESTINATION "${BIN_DIR}")
        endif(UNIX)

        message(${DllSTATUS} ${LibName} "  (HotSwap)")
    ELSE()
        message(${LibSTATUS} ${LibName})
    ENDIF(${Linkage} STREQUAL "SHARED")

ENDFUNCTION()

FUNCTION(CreateExecutable LibName LibSource Dependencis LibLinkFlags)

    link_directories(${PROJECTDEPS_PATH}/lib)

    add_executable(${LibName} ${LibSource} )

    IF(EAC)
        SETUP_EDIT_AND_CONTINUE("${LibName}" 1)
    ENDIF(EAC)

    target_link_libraries(${LibName}
        ${Dependencis}
    )

    if( UNIX )
        install(TARGETS ${LibName} RUNTIME DESTINATION "${BIN_DIR}")
    elseif( WIN32 )
        install(TARGETS ${LibName} RUNTIME DESTINATION "${BIN_DIR}")
    endif(UNIX)

    message(${BinSTATUS} ${LibName})
ENDFUNCTION()

FUNCTION(CreateCxxPch LibName Header Source)
    # Generate precompiled header
    if( PCH )
        message(${LibExSTATUS} "${LibName} Precomplined header")

        set(PRIVATE_PCH_HEADER ${Header})
        set(PRIVATE_PCH_SOURCE ${Source})

        ADD_CXX_PCH(${LibName} ${PRIVATE_PCH_HEADER} ${PRIVATE_PCH_SOURCE})
        # Old style
        #if(CMAKE_COMPILER_IS_GNUCXX)
        #	add_precompiled_header(${LibName} ${Header})
        #elseif(MSVC)
        #	add_cxx_pch(${LibName} ${Header} ${Source})
        #endif()
    endif()
ENDFUNCTION()

# Collects all subdirectoroies into the given variable,
# which is useful to include all subdirectories.
# Ignores full qualified directories listed in the variadic arguments.
#
# Use it like:
# CollectIncludeDirectories(
#   ${CMAKE_CURRENT_SOURCE_DIR}
#   COMMON_PUBLIC_INCLUDES
#   # Exclude
#   ${CMAKE_CURRENT_SOURCE_DIR}/PrecompiledHeaders
#   ${CMAKE_CURRENT_SOURCE_DIR}/Platform)
#
FUNCTION(CollectIncludeDirectories current_dir variable)
  list(FIND ARGN "${current_dir}" IS_EXCLUDED)
  #MESSAGE("Excluded ${current_dir} on ${ARGN}")
  if(IS_EXCLUDED EQUAL -1)
    list(APPEND ${variable} " ${current_dir}")
    file(GLOB SUB_DIRECTORIES ${current_dir}/*)
    foreach(SUB_DIRECTORY ${SUB_DIRECTORIES})
      if (IS_DIRECTORY ${SUB_DIRECTORY})
        CollectIncludeDirectories("${SUB_DIRECTORY}" "${variable}" "${ARGN}")
      endif()
    endforeach()
    set(${variable} ${${variable}} PARENT_SCOPE)
  endif()
ENDFUNCTION()

FUNCTION(AppendDefaultIncludeDir)
    CollectIncludeDirectories("${CMAKE_CURRENT_SOURCE_DIR}" PUBLIC_INCLUDES "${ARGN}")

    include_directories(
      ${CMAKE_CURRENT_SOURCE_DIR}
      ${PUBLIC_INCLUDES}
      ${CMAKE_SOURCE_DIR}
      ${CMAKE_SOURCE_DIR}/Irrlicht
      ${CMAKE_SOURCE_DIR}/Irrlicht/include
      ${CMAKE_SOURCE_DIR}/dep
      ${CMAKE_SOURCE_DIR}/dep/zlib
      ${CMAKE_BINARY_DIR}
      ${CMAKE_BINARY_DIR}/Dependencies
      ${CMAKE_BINARY_DIR}/Dependencies/include
    )

    IF(UNIX)
        include_directories(
            ${LIBDL_INCLUDE_DIR}
            ${BFD_INCLUDE_DIR}
          )
    ENDIF(UNIX)

ENDFUNCTION()
