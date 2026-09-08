/**
 * Copyright (c) 2014-present, The osquery authors
 *
 * This source code is licensed as defined by the LICENSE file found in the
 * root directory of this source tree.
 *
 * SPDX-License-Identifier: (Apache-2.0 OR GPL-2.0-only)
 */

#include "openframe_machine_id_provider.h"

#include <cstdlib>
#include <fstream>

#include <boost/algorithm/string/trim.hpp>
#include <glog/logging.h>

namespace osquery {

const std::string kOpenframeMachineIdHeader = "x-machine-id";

namespace {

// Must match the openframe-client's app-support dir on each platform.
std::string machineIdFilePath() {
#ifdef WIN32
  const char* program_data = std::getenv("ProgramData");
  std::string base =
      (program_data != nullptr) ? program_data : "C:\\ProgramData";
  return base + "\\OpenFrame\\machine_id";
#elif defined(__APPLE__)
  return "/Library/Application Support/OpenFrame/machine_id";
#else
  return "/var/lib/openframe/machine_id";
#endif
}

} // namespace

OpenframeMachineIdProvider& OpenframeMachineIdProvider::getInstance() {
  static OpenframeMachineIdProvider instance;
  return instance;
}

std::string OpenframeMachineIdProvider::getMachineId() {
  std::lock_guard<std::mutex> lock(mutex_);
  if (!machine_id_.empty()) {
    return machine_id_;
  }

  auto path = machineIdFilePath();
  std::string machine_id;
  std::ifstream machine_id_file(path);
  if (machine_id_file.is_open()) {
    std::getline(machine_id_file, machine_id);
    boost::algorithm::trim(machine_id);
  }

  if (machine_id.empty()) {
    if (!warned_) {
      LOG(WARNING) << "Could not read OpenFrame machine id from: " << path;
      warned_ = true;
    }
    return "";
  }

  machine_id_ = machine_id;
  LOG(INFO) << "OpenFrame machine id loaded from: " << path;
  return machine_id_;
}

} // namespace osquery
