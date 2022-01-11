cmake_minimum_required(VERSION 3.15)

project("ipc")

aux_source_directory(. SRCS)

if(WIN32)
	file(GLOB HEADER *.h*)

	include_directories(${PROJECT_SOURCE_DIR}/windowsinc)
	link_directories(${PROJECT_SOURCE_DIR}/windowslib)
	add_library(${PROJECT_NAME} ${SRCS} ${HEADER})
else(WIN32)
	find_package(Boost REQUIRED)
	add_definitions(-Wno-implicit-fallthrough)
	add_library(${PROJECT_NAME} SHARED ${SRCS})
	target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
endif(WIN32)
