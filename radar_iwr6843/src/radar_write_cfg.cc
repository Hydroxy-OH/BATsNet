#include <chrono>
#include <iostream>
#include <memory>
#include <thread>

#include <experimental/filesystem>

#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/text_format.h>

#include "iwr6843_read.h"
#include "proto/radar.pb.h"

const char *kTTY1 = "/dev/ttyACM0";
const char *kTTY2 = "/dev/ttyACM1";
const char *kTTY3 = "/dev/ttyACM2";
const char *kTTY4 = "/dev/ttyACM3";

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "[Usage] " << argv[0] << " conf_file" << std::endl;
    exit(1);
  }

  int fd = open(argv[1], O_RDONLY);
  radar::DeviceConf conf;
  google::protobuf::io::FileInputStream fstream(fd);
  if (!google::protobuf::TextFormat::Parse(&fstream, &conf)) {
    std::cerr << "Error while parsing conf" << std::endl;
    exit(1);
  }

  const char *control_port = kTTY1;
  const char *data_port = kTTY2;
  if (!std::experimental::filesystem::exists(control_port)) {
    control_port = kTTY3;
    data_port = kTTY4;
  }
  if (conf.has_control_tty()) {
    control_port = conf.control_tty().data();
  }
  if (conf.has_data_tty()) {
    data_port = conf.data_tty().data();
  }
  std::cout << "data_port: " << data_port << std::endl;
  std::cout << "control_port: " << control_port << std::endl;

  const auto &base_conf = conf.conf().base();
  const auto &base_state = conf.state().base();

  auto modes = conf.conf().possible_mode();
  const auto &cfg_file = modes.at(conf.state().current_mode());
  radar::Reader_IWR6843 reader(control_port, B115200, data_port, B921600);
  // // The Setup call will write cfg
  reader.Setup(cfg_file.c_str());
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  return 0;
}
