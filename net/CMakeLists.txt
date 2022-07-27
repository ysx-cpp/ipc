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
	add_library(${PROJECT_NAME} STATIC ${SRCS})
endif(WIN32)

if (TESTS)
	# target
	set(EXAMPLES "examples")
	add_definitions(-std=c++11 -g -pipe -fPIC -Wall -Wuninitialized -Wpessimizing-move)
	
	if (TEST_TCP)
		add_executable(${EXAMPLES} examples/testtcp.cpp)
	elseif(TEST_HTTP)
		add_executable(${EXAMPLES} examples/testhttp.cpp)
	endif()

	add_dependencies(${EXAMPLES} ${PROJECT_NAME})
	find_package(Boost REQUIRED)
	set(_BOOST_LIBRARIES_ boost_system boost_thread boost_filesystem boost_coroutine boost_regex z rt ssl crypto pthread)
	target_link_libraries(${PROJECT_NAME} ${_BOOST_LIBRARIES_})
	message("${PROJECT_NAME} ${Boost_LIBRARIES} ${_BOOST_LIBRARIES_}")
	target_link_libraries(${EXAMPLES} ${PROJECT_NAME} ${_BOOST_LIBRARIES_})
endif(TESTS)