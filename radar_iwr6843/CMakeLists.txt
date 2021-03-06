cmake_minimum_required( VERSION 3.9 )
project( radar_iwr6843 VERSION 0.0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Protobuf REQUIRED)
# find_package(Boost 1.40 COMPONENTS algorithm REQUIRED)
# if(Boost_FOUND)
#   include_directories(${Boost_INCLUDE_DIRS})
# endif()

include_directories(${Protobuf_INCLUDE_DIRS})
include_directories(${GLOBAL_HEADER})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})


EXEC_PROGRAM(protoc 
  ARGS --grpc_out=${CMAKE_CURRENT_SOURCE_DIR}/proto
       -I=${CMAKE_CURRENT_SOURCE_DIR}/proto
       -I=${GLOBAL_HEADER}/common/proto
       --cpp_out=${CMAKE_CURRENT_SOURCE_DIR}/proto
       --plugin=protoc-gen-grpc=`which grpc_cpp_plugin`
       --grpc_python_out=${CMAKE_CURRENT_SOURCE_DIR}/proto
       --python_out=${CMAKE_CURRENT_SOURCE_DIR}/proto
       --plugin=protoc-gen-grpc_python=`which grpc_python_plugin`
       ${CMAKE_CURRENT_SOURCE_DIR}/proto/*.proto)

if (INSTALL_DRIVER)
  file(GLOB PROTO_SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/proto/*.cc)
  file(GLOB CONF_FILES ${CMAKE_CURRENT_SOURCE_DIR}/conf/*)

  add_library(radar_lib STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/iwr6843_read.cc
    ${PROTO_SRC_FILES})
  target_link_libraries(radar_lib ${Protobuf_LIBRARIES}
    stdc++fs data_collector service_node transport common)

  add_executable(radar_iwr6843 ${CMAKE_CURRENT_SOURCE_DIR}/src/radar_iwr6843.cc)
  target_link_libraries(radar_iwr6843 radar_lib)
  
  add_executable(radar_write_cfg ${CMAKE_CURRENT_SOURCE_DIR}/src/radar_write_cfg.cc)
  target_link_libraries(radar_write_cfg radar_lib)

  add_library(radar_control_lib STATIC src/radar_control.cc src/iwr6843_control.cc)
  target_link_libraries(radar_control_lib radar_lib device)

  add_executable(radar_controller src/controller.cc)
  target_link_libraries(radar_controller radar_control_lib service_node)

  add_executable(debug ${CMAKE_CURRENT_SOURCE_DIR}/src/radar_cross_node_debug.cc)
  target_link_libraries(debug radar_lib)

  set(RADAR_CONF_INSTALL_DIR ${BATSNET_CONF_INSTALL_DIR}/radar)
  install(DIRECTORY DESTINATION ${RADAR_CONF_INSTALL_DIR})
  install(DIRECTORY conf/ DESTINATION ${RADAR_CONF_INSTALL_DIR})
  install(TARGETS radar_iwr6843 DESTINATION ${BATSNET_BIN_INSTALL_DIR})
  install(TARGETS radar_controller DESTINATION ${BATSNET_BIN_INSTALL_DIR})

  # install(FILES systemd/radar.service DESTINATION /etc/systemd/system)
endif(INSTALL_DRIVER)
