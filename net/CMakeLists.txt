cmake_minimum_required(VERSION 3.15)

project("net")

aux_source_directory(. SRCS)

if(WIN32)
	file(GLOB HEADER *.h*)

	include_directories(${PROJECT_SOURCE_DIR}/windowsinc)
	link_directories(${PROJECT_SOURCE_DIR}/windowslib)
	add_library(${PROJECT_NAME} ${SRCS} ${HEADER})
else(WIN32)
	find_package(Boost REQUIRED)
	add_definitions(-Wno-implicit-fallthrough)
	add_library(${PROJECT_NAME} STATIC ${SRCS})
	target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
endif(WIN32)

#if (BUILD_EXAMPLES)
# target
if (NOT Boost_LIBRARIES)
	set(Boost_LIBRARIES boost_system boost_thread boost_filesystem boost_regex z rt ssl crypto pthread ${Boost_LIBRARIES})
endif(Boost_LIBRARIES)

message("boost libs ------------- ${Boost_LIBRARIES}")
set(EXAMPLES "examples")
add_definitions(-std=c++11 -g -pipe -fPIC -Wall -Wuninitialized -Wpessimizing-move)
add_executable(${EXAMPLES} examples/example.cpp)
add_dependencies(${EXAMPLES} ${PROJECT_NAME})
target_link_libraries(${EXAMPLES} ${PROJECT_NAME} ${Boost_LIBRARIES})
#endif(BUILD_EXAMPLES)