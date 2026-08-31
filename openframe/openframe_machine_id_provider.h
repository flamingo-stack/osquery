/**
 * Copyright (c) 2014-present, The osquery authors
 *
 * This source code is licensed as defined by the LICENSE file found in the
 * root directory of this source tree.
 *
 * SPDX-License-Identifier: (Apache-2.0 OR GPL-2.0-only)
 */

#pragma once

#include <mutex>
#include <string>
#include <boost/noncopyable.hpp>

namespace osquery {

/// HTTP header carrying the locally generated OpenFrame machine id.
extern const std::string kOpenframeMachineIdHeader;

/**
 * @brief Provides the locally generated OpenFrame machine id
 *
 * Reads the machine id written by the openframe-client to the shared
 * OpenFrame data directory and caches it for the process lifetime.
 */
class OpenframeMachineIdProvider : private boost::noncopyable {
 public:
  /**
   * @brief Get the singleton instance of OpenframeMachineIdProvider
   *
   * @return Reference to the singleton instance
   */
  static OpenframeMachineIdProvider& getInstance();

  /**
   * @brief Get the machine id, reading it from disk on first success
   *
   * @return The machine id, or an empty string if not available yet
   */
  std::string getMachineId();

 private:
  OpenframeMachineIdProvider() = default;
  ~OpenframeMachineIdProvider() = default;

  std::mutex mutex_;
  std::string machine_id_;
  bool warned_ = false;
};

} // namespace osquery
