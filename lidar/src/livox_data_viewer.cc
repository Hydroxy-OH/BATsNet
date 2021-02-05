#include <chrono>
#include <fcntl.h>
#include <iostream>
#include <memory>
#include <thread>

#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/text_format.h>

#include "data_collector/include/data_io.h"

#include "proto/lidar.pb.h"

using DataType = livox::PointCloud;

constexpr int kBufSize = 1 << 21;

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "[Usage] " << argv[0] << " data_file" << std::endl;
    exit(1);
  }
  std::string data_file(argv[1]);
  data_collector::BufReader reader(data_file);

  char buf[kBufSize];
  int len = 0;
  while (reader.Read(buf, &len, kBufSize)) {
    DataType cloud;
    cloud.ParseFromArray(buf, len);
    std::cout << std::to_string(cloud.stamp())
              << " PointCount: " << cloud.point_size() << std::endl;
    //   std::cout << cloud.DebugString() << std::endl;
  }

  return 0;
}
