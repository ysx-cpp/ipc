cmake_minimum_required(VERSION 3.15)

project("shm")

aux_source_directory(. SRC)

if(WIN32)
	file(GLOB HEADER *.h*)
	add_library(${PROJECT_NAME} ${SRC} ${HEADER})
else(WIN32)
	find_package(Boost REQUIRED)
	add_definitions(-Wno-implicit-fallthrough)
	add_library(${PROJECT_NAME} SHARED ${SRC})
	target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
endif(WIN32)
