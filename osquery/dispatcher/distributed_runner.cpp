/**
 * Copyright (c) 2014-present, The osquery authors
 *
 * This source code is licensed as defined by the LICENSE file found in the
 * root directory of this source tree.
 *
 * SPDX-License-Identifier: (Apache-2.0 OR GPL-2.0-only)
 */

#include <chrono>
#include <string>

#include <osquery/core/flags.h>
#include <osquery/core/system.h>
#include <osquery/database/database.h>
#include <osquery/distributed/distributed.h>
#include <osquery/logger/logger.h>

#include <osquery/dispatcher/distributed_runner.h>
#include <osquery/utils/conversions/tryto.h>
#include <osquery/utils/system/time.h>

namespace osquery {

FLAG(uint64,
     distributed_interval,
     60,
     "Seconds between polling for new queries (default 60)")

DECLARE_bool(disable_distributed);
DECLARE_bool(openframe_mode);
DECLARE_string(distributed_plugin);

const size_t kDistributedAccelerationInterval = 5;

static void logOutcomeChange(const char* operation,
                             const Status& status,
                             std::string& last) {
  const auto message = status.ok() ? std::string() : status.getMessage();
  if (message == last) {
    return;
  }

  last = message;
  if (status.ok()) {
    LOG(INFO) << operation << ": recovered";
  } else {
    LOG(ERROR) << operation << ": " << message;
  }
}

void DistributedRunner::start() {
  auto dist = Distributed();
  std::string last_read_error;
  std::string last_write_error;

  while (!interrupted()) {
    const auto read_status = dist.pullUpdates();
    const auto write_status = dist.runQueries();
    if (FLAGS_openframe_mode) {
      logOutcomeChange(
          "Reading distributed queries", read_status, last_read_error);
      logOutcomeChange(
          "Writing distributed query results", write_status, last_write_error);
    }

    dist.cleanupExpiredRunningQueries();

    std::string accelerate_checkins_expire_str = "-1";
    Status status = getDatabaseValue(kPersistentSettings,
                                     "distributed_accelerate_checkins_expire",
                                     accelerate_checkins_expire_str);
    if (!status.ok() || getUnixTime() > tryTo<unsigned long int>(
                                            accelerate_checkins_expire_str, 10)
                                            .takeOr(0ul)) {
      pause(std::chrono::seconds(FLAGS_distributed_interval));
    } else {
      pause(std::chrono::seconds(kDistributedAccelerationInterval));
    }
  }
}

Status startDistributed() {
  if (!FLAGS_disable_distributed) {
    Dispatcher::addService(std::make_shared<DistributedRunner>());
    return Status::success();
  } else {
    return Status(1, "Distributed query service not enabled.");
  }
}
} // namespace osquery
