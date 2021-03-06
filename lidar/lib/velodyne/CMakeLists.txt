cmake_minimum_required( VERSION 3.9 )
project( velodyne VERSION 0.0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Protobuf REQUIRED)
# find_package(Boost 1.40 COMPONENTS algorithm REQUIRED)
# if(Boost_FOUND)
#   include_directories(${Boost_INCLUDE_DIRS})
# endif()

# include_directories(${Protobuf_INCLUDE_DIRS})


EXEC_PROGRAM(protoc 
  ARGS -I=${CMAKE_CURRENT_SOURCE_DIR}/proto
       --cpp_out=${CMAKE_CURRENT_SOURCE_DIR}/proto
       --python_out=${CMAKE_CURRENT_SOURCE_DIR}/proto
       ${CMAKE_CURRENT_SOURCE_DIR}/proto/*.proto)

add_library(velodyne_proto SHARED proto/velodyne.pb.cc)
target_link_libraries(velodyne_proto PUBLIC ${Protobuf_LIBRARIES})
target_include_directories(velodyne_proto PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/proto)

add_subdirectory(velodyne_driver)
add_subdirectory(velodyne_pointcloud)

install(TARGETS velodyne_proto DESTINATION ${BATSNET_LIB_INSTALL_DIR})
