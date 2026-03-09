#include "openframe_machine_id_provider.h"

#include <fstream>
#include <sstream>

#include <boost/algorithm/string/trim.hpp>
#include <glog/logging.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <cstdlib>
#endif

namespace osquery {

OpenframeMachineIdProvider& OpenframeMachineIdProvider::getInstance() {
  static OpenframeMachineIdProvider instance;
  return instance;
}

std::string OpenframeMachineIdProvider::getMachineId() {
  if (!initialized_) {
    cached_machine_id_ = readFromFile();
    initialized_ = true;
  }
  return cached_machine_id_;
}

void OpenframeMachineIdProvider::refresh() {
  cached_machine_id_ = readFromFile();
  initialized_ = true;
}

std::string OpenframeMachineIdProvider::getFilePath() {
#ifdef _WIN32
  char* programData = std::getenv("ProgramData");
  if (programData == nullptr) {
    return "";
  }
  return std::string(programData) + "\\OpenFrame\\machine_id";
#elif defined(__APPLE__)
  return "/Library/Application Support/OpenFrame/machine_id";
#else
  return "/var/lib/openframe/machine_id";
#endif
}

std::string OpenframeMachineIdProvider::readFromFile() {
  std::string path = getFilePath();
  if (path.empty()) {
    LOG(WARNING) << "Could not determine machine_id file path";
    return "";
  }

  std::ifstream file(path);
  if (!file.is_open()) {
    VLOG(1) << "Could not open machine_id file: " << path;
    return "";
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  std::string content = buffer.str();
  boost::algorithm::trim(content);

  if (content.empty()) {
    LOG(WARNING) << "Machine ID file is empty: " << path;
    return "";
  }

  VLOG(1) << "Read machine ID from " << path;
  return content;
}

} // namespace osquery
