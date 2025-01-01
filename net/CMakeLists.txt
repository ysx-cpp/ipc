cmake_minimum_required(VERSION 3.15)

project("net")

aux_source_directory(. SRCS)

if(WIN32)
        # vcpkg path
        set(VCPKG_DIR "F:/client/external/vcpkg")
        set(CMAKE_TOOLCHAIN_FILE "${VCPKG_DIR}/scripts/buildsystems/vcpkg.cmake")
        set(CMAKE_PREFIX_PATH ${VCPKG_DIR}/installed/x64-windows)

        # 设置编译器
        set(Boost_COMPILER "MSVC")
        set(BOOST_DETECTED_TOOLSET "MSVC")

        # boost 安装路径
        set(BOOST_ROOT ${CMAKE_PREFIX_PATH})
        #set(Boost_DIR ${THIRD_PARTY_DIR}/boost/lib/cmake/Boost-1.71.0)
        set(Boost_INCLUDE_DIR ${BOOST_ROOT}/include)
        set(Boost_LIBRARY_DIRS ${BOOST_ROOT}/lib)

        # 查找 Boost 包
        find_package(Boost REQUIRED COMPONENTS system filesystem thread coroutine context regex)
        message(STATUS "Boost_INCLUDE_DIR: ${Boost_INCLUDE_DIR}")
        message(STATUS "Boost_LIBRARY_DIRS: ${Boost_LIBRARY_DIRS}")
        message(STATUS "Boost_LIBRARIES: ${Boost_LIBRARIES}")

        include_directories(${Boost_INCLUDE_DIR})
        link_directories(${Boost_LIBRARIES})
	
	file(GLOB HEADER *.h*)
        add_library(${PROJECT_NAME} STATIC ${SRCS} ${HEADER})
        #add_library(${PROJECT_NAME} SHARED ${SRCS} ${HEADER})

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

