cmake_minimum_required(VERSION 3.30)
project(libswitch C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64")

add_definitions(
	-DXPLM200
	-DXPLM210
	-DXPLM300
	-DXPLM301
	-DXPLM302
	-DXPLM303
)

add_library(libswitch SHARED
	libswitch.c
	libswitch.h
)

target_include_directories(libswitch
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/src

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
elseif (APPLE)
	# Compiler Definitions
	add_definitions(-DAPL)

	# Libraries to link
	target_link_libraries(libswitch
		${CMAKE_CURRENT_SOURCE_DIR}/deps/XPSDK/Libraries/Mac/XPLM.framework
		${CMAKE_CURRENT_SOURCE_DIR}/deps/XPSDK/Libraries/Mac/XPWidgets.framework
	)
else ()
	# Compiler Definitions
	add_definitions(-DLIN)
endif ()