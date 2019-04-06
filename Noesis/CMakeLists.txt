SET(MODUL_LINKAGE "SHARED")
SET(MODUL_LINK_LIBS "")
SET(MODUL_LINK_FLAGS "")

file(GLOB_RECURSE LibCommon_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp *.h *.c *.cc)

if (BUILD_HAS_RESOURCE_SYSTEM)
	group_source(${LibCommon_SRCS})
endif (BUILD_HAS_RESOURCE_SYSTEM)

CreateLibrary("NoesisApp" "${LibCommon_SRCS}" "${MODUL_LINKAGE}" "${MODUL_LINK_LIBS}" "${MODUL_LINK_FLAGS}")

target_link_libraries(NoesisApp
PRIVATE
   CFramework
   irrlicht
PUBLIC
   noesisgui
)

target_include_directories(NoesisApp
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}
        ${PROTOBUF_INCLUDE_DIRS}
        ${CMAKE_CURRENT_BINARY_DIR}/.Assembly
)

target_compile_definitions(NoesisApp
    PUBLIC  NS_IRR_NOESISAPP_API
    PRIVATE TC_API_EXPORT_NOESISAPP)

set_target_properties(NoesisApp
    PROPERTIES
      FOLDER
        "Contrib")