#include "include/data_io.h"

#include <fcntl.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <lz4.h>

#include <chrono>
#include <experimental/filesystem>
#include <iostream>
#include <thread>

namespace fs = std::experimental::filesystem;

namespace data_collector {
BufFolderLock::BufFolderLock(const std::string &folder) {
  std::string lock_file = folder + "/lock.lck";
  mode_t m = umask(0);
  fd_ = open(lock_file.c_str(), O_RDWR | O_CREAT, 0666);
  umask(0);
  if (fd_ <= 0) {
    std::cout << "Failed to Lock folder " << folder
              << " , check whether the folder path exist" << std::endl;
    exit(1);
  }

  while (flock(fd_, LOCK_EX | LOCK_NB) != 0) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Waiting folder lock " << lock_file << std::endl;
  }
}

BufFolderLock::~BufFolderLock() {
  if (fd_ > 0) {
    flock(fd_, LOCK_UN);
  }
}

std::string GetNowTimeStramp() {
  time_t rawtime;
  struct tm *timeinfo;
  char buffer[80];
  time(&rawtime);
  timeinfo = localtime(&rawtime);
  strftime(buffer, sizeof(buffer), "%Y%m%d%H%M%S", timeinfo);
  return std::string(buffer);
}

BufWriter::BufWriter(const std::string &topic) {
  std::string folder_name = topic;
  for (auto &w : folder_name) {
    if (w == '/') {
      w = '#';
    }
  }
  std::string sname = std::string(kBufFolder) + "/" + folder_name;
  fs::create_directories(sname);
  std::string name = std::string(kBufTmpFolder) + "/" + folder_name;
  fs::create_directories(name);
  fname_ = name + "/" + GetNowTimeStramp();
  sfname_ = sname + "/" + GetNowTimeStramp();
  f_ = gzopen(fname_.c_str(), "w2");
}

BufWriter::~BufWriter() {
  gzclose(f_);
  fs::rename(fname_, sfname_);
}

void BufWriter::Write(const char *buf, const int len) {
  if (buf_.size() < len) {
    buf_.resize(len);
  }
  int32_t wlen = len;
  gzwrite(f_, reinterpret_cast<char *>(&wlen), sizeof(int32_t));
  gzwrite(f_, buf, len);
}

BufReader::BufReader(const std::string &file) : fname_(file) {
  f_ = gzopen(file.c_str(), "r");
}

bool BufReader::Read(char *buf, int *len, const int buf_size) {
  int32_t msg_len = 0;
  gzread(f_, reinterpret_cast<char *>(&msg_len), sizeof(int32_t));
  *len = msg_len;
  if (msg_len > 0) {
    gzread(f_, buf, msg_len);
  }
  return !gzeof(f_);
}

void BufReader::RemoveBuf() { 
  gzclose(f_);
  fs::remove(fname_);
}

} // namespace data_collector
