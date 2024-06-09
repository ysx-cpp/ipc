cmake_minimum_required(VERSION 3.15)

project("shm")

aux_source_directory(. SRC)

if(WIN32)
	file(GLOB HEADER *.h*)
	add_library(${PROJECT_NAME} ${SRC} ${HEADER})
else(WIN32)
	add_definitions(-Wno-implicit-fallthrough)
	if(OPT_BUILD_SHARDE)
		find_package(Boost REQUIRED)
		add_library(${PROJECT_NAME} SHARED ${SRC})
		target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
	else(OPT_BUILD_SHARDE)
		add_library(${PROJECT_NAME} STATIC ${SRC})
	endif(OPT_BUILD_SHARDE)
endif(WIN32)
