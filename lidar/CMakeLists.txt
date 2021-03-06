cmake_minimum_required( VERSION 3.9 )
project( lidar VERSION 0.0.1 LANGUAGES C CXX)

add_subdirectory(lib)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Protobuf REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})


EXEC_PROGRAM(protoc 
  ARGS --grpc_out=${CMAKE_CURRENT_SOURCE_DIR}/proto
       -I=${CMAKE_CURRENT_SOURCE_DIR}/proto
       -I=${GLOBAL_HEADER}/common/proto
       -I=${CMAKE_CURRENT_SOURCE_DIR}/lib/velodyne/proto
       -I=${CMAKE_CURRENT_SOURCE_DIR}/lib/rslidar/proto
       -I=${CMAKE_CURRENT_SOURCE_DIR}/lib/livox/proto
       --cpp_out=${CMAKE_CURRENT_SOURCE_DIR}/proto
       --plugin=protoc-gen-grpc=`which grpc_cpp_plugin`
       --grpc_python_out=${CMAKE_CURRENT_SOURCE_DIR}/proto
       --python_out=${CMAKE_CURRENT_SOURCE_DIR}/proto
       --plugin=protoc-gen-grpc_python=`which grpc_python_plugin`
       ${CMAKE_CURRENT_SOURCE_DIR}/proto/*.proto)
file(GLOB PROTO_SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/proto/*.cc)

if(INSTALL_DRIVER)
    add_library(lidar_proto STATIC ${PROTO_SRC_FILES})
    target_link_libraries(lidar_proto ${Protobuf_LIBRARIES}
        common rslidar_proto velodyne_proto livox_lidar_proto)
    
    add_executable(rslidar src/rslidar.cc)
    target_link_libraries(rslidar rslidar_lib 
        lidar_proto ${Protobuf_LIBRARIES} data_collector transport service_node)

    add_executable(velodyne src/velodyne.cc)
    target_link_libraries(velodyne velodyne_driver 
        lidar_proto ${Protobuf_LIBRARIES} data_collector transport service_node)
    
    add_executable(livox src/livox.cc)
    target_link_libraries(livox livox_lidar_lib
        lidar_proto ${Protobuf_LIBRARIES} data_collector transport service_node)

    add_library(lidar_control_lib STATIC src/lidar_control.cc src/rslidar_control.cc src/velodyne_control.cc src/livox_control.cc)
    target_link_libraries(lidar_control_lib device lidar_proto)

    add_executable(lidar_controller src/controller.cc)
    target_link_libraries(lidar_controller lidar_control_lib service_node)

    ## For DEBUG
    add_executable(livox_data_viewer src/livox_data_viewer.cc)
    target_link_libraries(livox_data_viewer livox_lidar_lib
    lidar_proto ${Protobuf_LIBRARIES} data_collector transport service_node)

    set(LIDAR_CONF_INSTALL_DIR ${BATSNET_CONF_INSTALL_DIR}/lidar)
    install(DIRECTORY DESTINATION ${LIDAR_CONF_INSTALL_DIR})
    install(DIRECTORY conf/ DESTINATION ${LIDAR_CONF_INSTALL_DIR})

    install(TARGETS rslidar DESTINATION ${BATSNET_BIN_INSTALL_DIR})
    install(TARGETS velodyne DESTINATION ${BATSNET_BIN_INSTALL_DIR})
    install(TARGETS livox DESTINATION ${BATSNET_BIN_INSTALL_DIR})
    install(TARGETS lidar_controller DESTINATION ${BATSNET_BIN_INSTALL_DIR})

    install(FILES systemd/lidar.service DESTINATION /etc/systemd/system)
endif(INSTALL_DRIVER)
