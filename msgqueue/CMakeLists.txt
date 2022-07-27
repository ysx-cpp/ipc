cmake_minimum_required(VERSION 3.15)
project("msgqueue")

find_package(Boost REQUIRED  filesystem system regex)
find_package(Protobuf REQUIRED)
find_package(Glog REQUIRED)
find_package(ZeroMQ REQUIRED)

if (NOT ZeroMQ_FOUND)
    # load in pkg-config support
    find_package(PkgConfig)
    # use pkg-config to get hints for 0mq locations
    pkg_check_modules(PC_ZeroMQ QUIET zmq)

    find_library(ZeroMQ_LIBRARY
        NAMES zmq
        PATHS ${PC_ZeroMQ_LIBRARY_DIRS}
    )
endif()

file(GLOB PROTO_FILES "proto/*.proto")
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS "${PROTO_FILES}")
list(GET PROTO_HDRS 0 PROTO_HDR)
get_filename_component(PROTO_DIR ${PROTO_HDR} PATH)

include_directories("${PROTO_DIR}" 
    ${PROJECT_SOURCE_DIR}/src
)

#target_include_directories(${PROJECT_NAME} PUBLIC ${ZeroMQ_INCLUDE_DIR})
set(LIBS ${ZeroMQ_LIBRARY} ${PROTOBUF_LIBRARIES} ${Boost_LIBRARIES} ${GLOG_LIBRARIES} glog)

file(GLOB_RECURSE SRCS src/*.cpp)
add_library(${PROJECT_NAME} SHARED ${SRCS} ${PROTO_SRCS})
target_link_libraries(${PROJECT_NAME} ${LIBS})

# target
set(name "middleware_server")
add_executable(${name} server_node.cpp)
add_dependencies(${name} ${PROJECT_NAME})
target_link_libraries(${name} ${PROJECT_NAME} ${LIBS})

# target
set(name "middleware_debugger")
add_executable(${name} examples/ipc_debugger.cpp)
add_dependencies(${name} ${PROJECT_NAME})
target_link_libraries(${name} ${PROJECT_NAME} ${LIBS})

set(targets ${PROJECT_NAME} middleware_server middleware_debugger)

install(TARGETS ${targets}
    ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
    LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
    RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)

#install(DIRECTORY 
#    include
#    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
#    USE_SOURCE_PERMISSIONS
#)

#install(DIRECTORY
#    config
#    DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}
#    USE_SOURCE_PERMISSIONS
#)