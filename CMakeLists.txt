project(cruiqt)

cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules/")

set(FILEVERSION_V	"1, 0, 0, 0")
set(PRODUCTVERSION_V	"1, 0, 0, 0")
set(VERSION		"1.0.0")
set(PACKAGE_DATADIR	${CMAKE_INSTALL_PREFIX}/share)

option(USE_DEBUG_EXTRA_OPTS "Add extra debug flags and technique" OFF)

configure_file(config.h.cmake ${CMAKE_BINARY_DIR}/config.h)

# Find the Qt5 libraries
find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)

include(Qt5TranslationToolsMacros)

# crengine-ng
find_package(crengine-ng REQUIRED)

# Add include path for config.h
include_directories(${CMAKE_BINARY_DIR})

# force use exceptions
set(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS} -fexceptions")
# add debug defines
set(CMAKE_C_FLAGS_DEBUG     "${CMAKE_C_FLAGS_DEBUG}     -D_DEBUG")
set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   -D_DEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG")

if(USE_DEBUG_EXTRA_OPTS)
	add_definitions(-D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC)
	set(DEBUG_EXTRA_FLAGS "-fstack-check -fstack-protector -fno-omit-frame-pointer -Wshift-overflow=2")
	if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
		set (DEBUG_EXTRA_FLAGS "${DEBUG_EXTRA_FLAGS} -fsanitize=address -fsanitize=undefined -fno-sanitize-recover -fno-sanitize=alignment")
		set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -fsanitize=address -fsanitize=undefined -fno-sanitize-recover -fstack-check -fstack-protector")
		set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -fsanitize=address -fsanitize=undefined -fno-sanitize-recover -fstack-check -fstack-protector")
	endif ("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${DEBUG_EXTRA_FLAGS}")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${DEBUG_EXTRA_FLAGS}")
endif(USE_DEBUG_EXTRA_OPTS)

if (WIN32)
	set(CMAKE_RC_COMPILER windres)
	# set rc syntax
	set(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> <FLAGS> <DEFINES> -O coff -o <OBJECT> -i <SOURCE>")
	set(CMAKE_RC_SOURCE_FILE_EXTENSIONS rc)
	set(CMAKE_RC_FLAGS "-I${CMAKE_BINARY_DIR}")
	# enable resource language for this project
	enable_language(RC)
endif(WIN32)

add_subdirectory(src)
