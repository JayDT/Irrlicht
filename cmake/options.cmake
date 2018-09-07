SET(CMAKE_COLOR_MAKEFILE   ON)
SET(CMAKE_VERBOSE_MAKEFILE OFF)
SET(CMAKE_TIMED_MAKEFILE   0)

option(BUILD_IN_USE_GLSLANG "Use buildin Glslang"                  ON)
option(BUILD_IN_DXC         "Use buildin HLSL compiler"            OFF)
option(PCH                "Use precompiled headers"                ON)
option(MEMCHECK           "Use Internal Memory checker"            OFF)
OPTION(BUILD_EXAMPLES     "Build examples." 	                   ON)
option(INSTEX             "Detect highest instruction Extension"   OFF)
option(DYNLIB             "Optimize for Dynamic Libraries"         OFF)

message("Loaded Config")
