cmake_minimum_required(VERSION 3.15)
project("messages")

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

if (NOT Glog_FOUND)
    # load in pkg-config support
    find_package(PkgConfig)
    # use pkg-config to get hints for 0mq locations
    pkg_check_modules(PC_Glog QUIET glog)

    find_library(Glog_LIBRARY
        NAMES glog
        PATHS ${PC_Glog_LIBRARY_DIRS}
    )
endif()

file(GLOB PROTO_FILES "protos/*.proto")
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS "${PROTO_FILES}")
list(GET PROTO_HDRS 0 PROTO_HDR)
get_filename_component(PROTO_DIR ${PROTO_HDR} PATH)

include_directories(${PROTO_DIR} ${PROJECT_SOURCE_DIR}/src)
set(LIBS ${ZeroMQ_LIBRARY} ${PROTOBUF_LIBRARIES} ${GLOG_LIBRARIES} glog)

# message("Glog_LIBRARY:${Glog_LIBRARY} GLOG_LIBRARIES:${GLOG_LIBRARIES} PROTOBUF_LIBRARIES:${PROTOBUF_LIBRARIES} ZeroMQ_LIBRARY:${ZeroMQ_LIBRARY}")

file(GLOB_RECURSE SRCS src/*.cpp)
add_library(${PROJECT_NAME} SHARED ${SRCS} ${PROTO_SRCS})
target_link_libraries(${PROJECT_NAME} ${LIBS})

# target
set(TEST_PROACTIVE "test_subscriber")
add_executable(${TEST_PROACTIVE} examples/test_subscriber.cpp)
add_dependencies(${TEST_PROACTIVE} ${PROJECT_NAME})
target_link_libraries(${TEST_PROACTIVE} ${PROJECT_NAME} ${LIBS})

# target
set(TEST_PASSIVE "test_publisher")
add_executable(${TEST_PASSIVE} examples/test_publisher.cpp)
add_dependencies(${TEST_PASSIVE} ${PROJECT_NAME})
target_link_libraries(${TEST_PASSIVE} ${PROJECT_NAME} ${LIBS})

set(targets ${PROJECT_NAME} ${TEST_PROACTIVE} ${TEST_PASSIVE})

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