# The ZLIB license
#
# Copyright (c) 2015 André Netzeband
#
# This software is provided 'as-is', without any express or implied
# warranty. In no event will the authors be held liable for any damages
# arising from the use of this software.
#
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely, subject to the following restrictions:
#
# 1. The origin of this software must not be misrepresented; you must not
#    claim that you wrote the original software. If you use this software
#    in a product, an acknowledgement in the product documentation would be
#    appreciated but is not required.
# 2. Altered source versions must be plainly marked as such, and must not be
#    misrepresented as being the original software.
# 3. This notice may not be removed or altered from any source distribution.
#

SET(CMAKE_MODULE_PATH 
	${CMAKE_MODULE_PATH} 
	${CMAKE_CURRENT_LIST_DIR}/Dependencies
	${CMAKE_CURRENT_LIST_DIR}/Compiler
	${CMAKE_CURRENT_LIST_DIR}/OperatingSystems
	${CMAKE_CURRENT_LIST_DIR}/Options
	${CMAKE_CURRENT_LIST_DIR}/Packages
)

FUNCTION(BUILD_APP_GENERIC APP_NAME APP_SOURCE_FILES APP_HEADER_FILES APP_INSTALL_FILES, APP_INSTALL_DIRS APP_IS_WIN32 APP_INSTALL_PATH)
	
    SET(EXECUTABLE_LINK_LIBS "")
    SET(EXECUTABLE_SRCS "")
    SET(EXECUTABLE_LINK_FLAGS ${CMAKE_EXE_LINKER_FLAGS_DEBUG})

    list(APPEND EXECUTABLE_LINK_LIBS
        irrlicht
    )

    list(APPEND EXECUTABLE_SRCS
        ${APP_SOURCE_FILES}
        ${APP_HEADER_FILES}
    )

    AppendDefaultIncludeDir()
    CreateExecutable("${APP_NAME}" "${EXECUTABLE_SRCS}" "${EXECUTABLE_LINK_LIBS}" "${EXECUTABLE_LINK_FLAGS}")

    set_target_properties(${APP_NAME}
        PROPERTIES
          FOLDER
            "Example")

ENDFUNCTION()

FUNCTION(BUILD_EXAMPLE EXAMPLE_NAME EXAMPLE_SOURCE_FILES EXAMPLE_HEADER_FILES EXAMPLE_INSTALL_FILES, EXAMPLE_INSTALL_DIRS)
	BUILD_APP_GENERIC("${EXAMPLE_NAME}" "${EXAMPLE_SOURCE_FILES}" "${EXAMPLE_HEADER_FILES}" "${EXAMPLE_INSTALL_FILES}" "${EXAMPLE_INSTALL_DIRS}" FALSE "example/${EXAMPLE_NAME}")
ENDFUNCTION()

FUNCTION(BUILD_WIN32_EXAMPLE EXAMPLE_NAME EXAMPLE_SOURCE_FILES EXAMPLE_HEADER_FILES EXAMPLE_INSTALL_FILES, EXAMPLE_INSTALL_DIRS)
	BUILD_APP_GENERIC("${EXAMPLE_NAME}" "${EXAMPLE_SOURCE_FILES}" "${EXAMPLE_HEADER_FILES}" "${EXAMPLE_INSTALL_FILES}" "${EXAMPLE_INSTALL_DIRS}" TRUE "example/${EXAMPLE_NAME}")
ENDFUNCTION()

FUNCTION(BUILD_TOOL TOOL_NAME TOOL_SOURCE_FILES TOOL_HEADER_FILES TOOL_INSTALL_FILES, TOOL_INSTALL_DIRS)
	BUILD_APP_GENERIC("${TOOL_NAME}" "${TOOL_SOURCE_FILES}" "${TOOL_HEADER_FILES}" "${TOOL_INSTALL_FILES}" "${TOOL_INSTALL_DIRS}" FALSE "tools/${TOOL_NAME}")
ENDFUNCTION()
