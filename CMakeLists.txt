cmake_minimum_required(VERSION 3.21)
project(libswitch C)

set(CMAKE_C_STANDARD 17)

# CMake Settings
set(CMAKE_C_FLAGS "-O3 -fPIC -pedantic -Wall")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64")

# Definitions for the X-Plane SDK
add_definitions(
	-DXPLM200
	-DXPLM210
	-DXPLM300
	-DXPLM301
	-DXPLM302
	-DXPLM303
)

# Include source code
add_library(libswitch
	${CMAKE_CURRENT_SOURCE_DIR}/libswitch.c
	${CMAKE_CURRENT_SOURCE_DIR}/libswitch.h
)

# Include directories for both libswitch & its dependencies
target_include_directories(libswitch
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}

	PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/deps/XPSDK/CHeaders/XPLM
	${CMAKE_CURRENT_SOURCE_DIR}/deps/XPSDK/CHeaders/Widgets
)

# OS-specific options
if (WIN32)
	# Compiler Definitions
	add_definitions(
		-DIBM
		-D_USE_MATH_DEFINES
	)

	# Libraries to link
	target_link_libraries(libswitch
		${CMAKE_CURRENT_SOURCE_DIR}/deps/XPSDK/Libraries/Win/XPLM_64.lib
		${CMAKE_CURRENT_SOURCE_DIR}/deps/XPSDK/Libraries/Win/XPWidgets_64.lib
	)

	# Output Directory
	set_target_properties(libswitch PROPERTIES
		ARCHIVE_OUTPUT_DIRECTORY ./build/win
	)
elseif (APPLE)
	# Compiler Definitions
	add_definitions(-DAPL)

	# Libraries to link
	target_link_libraries(libswitch
		${CMAKE_CURRENT_SOURCE_DIR}/deps/XPSDK/Libraries/Mac/XPLM.framework
		${CMAKE_CURRENT_SOURCE_DIR}/deps/XPSDK/Libraries/Mac/XPWidgets.framework
	)

	# Output Directory
	set_target_properties(libswitch PROPERTIES
		ARCHIVE_OUTPUT_DIRECTORY ./build/mac
	)
else ()
	# Compiler Definitions
	add_definitions(-DLIN)

	# Output Directory
	set_target_properties(libswitch PROPERTIES
		ARCHIVE_OUTPUT_DIRECTORY ./build/lin
	)
endif ()