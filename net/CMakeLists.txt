cmake_minimum_required(VERSION 3.15)

project(${LIBNET})

aux_source_directory(. SRCS)

if(WIN32)
    # vcpkg path
    message("VCPKG_ROOT:${VCPKG_ROOT}")

    # 设置编译器
    # set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
    set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "-shared")
    set(Boost_COMPILER "MSVC")
    set(BOOST_DETECTED_TOOLSET "MSVC")

    # 查找 Boost 包
    find_package(Boost REQUIRED COMPONENTS system filesystem thread coroutine context regex)
    message(STATUS "Boost_INCLUDE_DIR: ${Boost_INCLUDE_DIR}")
    message(STATUS "Boost_LIBRARY_DIRS: ${Boost_LIBRARY_DIRS}")
    message(STATUS "Boost_LIBRARIES: ${Boost_LIBRARIES}")

    include_directories(${Boost_INCLUDE_DIR})
    link_directories(${Boost_LIBRARIES})
	
    file(GLOB HEADER *.h*)
    add_library(${PROJECT_NAME} ${SRCS} ${HEADER})

    set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "${PROJECT_NAME}")
    target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

    # 链接 Boost 库
    target_include_directories(${PROJECT_NAME} PRIVATE ${Boost_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${Boost_LIBRARIES})

else(WIN32)

    add_definitions(-Wno-implicit-fallthrough)
    add_library(${PROJECT_NAME} STATIC ${SRCS})

    if (TESTS)
        # target
        set(EXAMPLES "examples")
        add_definitions(-std=c++11 -g -pipe -fPIC -Wall -Wuninitialized -Wpessimizing-move)

        #find_package(Boost REQUIRED)
        set(_BOOST_LIBRARIES_ boost_system boost_thread boost_filesystem boost_coroutine boost_context boost_regex z rt ssl crypto pthread)
        message("${PROJECT_NAME} ${Boost_LIBRARIES} ${_BOOST_LIBRARIES_}")

        if (TEST_TCP)
        add_executable(${EXAMPLES} examples/testtcp.cpp)
        elseif(TEST_HTTP)
                add_executable(${EXAMPLES} examples/testhttp.cpp)
        endif()

        target_link_libraries(${EXAMPLES} ${PROJECT_NAME} ${_BOOST_LIBRARIES_})
        add_dependencies(${EXAMPLES} ${PROJECT_NAME})
    endif(TESTS)
endif(WIN32)

