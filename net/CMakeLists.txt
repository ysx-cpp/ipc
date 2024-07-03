cmake_minimum_required(VERSION 3.15)

project("net")

aux_source_directory(. SRCS)

if(WIN32)
	file(GLOB HEADER *.h*)

	include_directories(${PROJECT_SOURCE_DIR}/windowsinc)
	link_directories(${PROJECT_SOURCE_DIR}/windowslib)
	add_library(${PROJECT_NAME} ${SRCS} ${HEADER})
else(WIN32)
	add_definitions(-Wno-implicit-fallthrough)
	add_library(${PROJECT_NAME} SHARED ${SRCS})
	set(_BOOST_LIBRARIES_ boost_system boost_thread boost_filesystem boost_regex z rt ssl crypto pthread)
	target_link_libraries(${PROJECT_NAME} ${_BOOST_LIBRARIES_})
	message("${PROJECT_NAME} ${OpenSSL_LIBRARIES} ${Boost_LIBRARIES} ${_BOOST_LIBRARIES_}")
endif(WIN32)

if (TESTS)
# target
set(EXAMPLES "examples")
add_definitions(-std=c++11 -g -pipe -fPIC -Wall -Wuninitialized -Wpessimizing-move)
add_executable(${EXAMPLES} examples/example.cpp)
add_dependencies(${EXAMPLES} ${PROJECT_NAME})
target_link_libraries(${EXAMPLES} ${PROJECT_NAME} ${_BOOST_LIBRARIES_})
endif(TESTS)