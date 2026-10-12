/**
 * Copyright (c) 2014-present, The osquery authors
 *
 * This source code is licensed as defined by the LICENSE file found in the
 * root directory of this source tree.
 *
 * SPDX-License-Identifier: (Apache-2.0 OR GPL-2.0-only)
 */

#include <mutex>

#include <osquery/core/flags.h>
#include <osquery/events/linux/bpf/setrlimit.h>

#include <sys/resource.h>

namespace osquery {

DECLARE_bool(enable_bpf_events);

namespace {
std::once_flag setrlimit_flag;
bool setrlimit_succeeded{false};

bool configureBPFMemoryLimitsHelper() {
  if (!FLAGS_enable_bpf_events) {
    return true;
  }

  struct rlimit rl = {};
  rl.rlim_max = RLIM_INFINITY;
  rl.rlim_cur = rl.rlim_max;

  auto err = setrlimit(RLIMIT_MEMLOCK, &rl);
  if (err != 0) {
    return false;
  }

  return true;
}
} // namespace

Status configureBPFMemoryLimits() {
  std::call_once(setrlimit_flag, [&]() {
    setrlimit_succeeded = configureBPFMemoryLimitsHelper();
  });

  if (!setrlimit_succeeded) {
    return Status::failure(
        "Failed to setup the memory lock limits. The BPF tables may not work "
        "correctly.");
  }

  return Status::success();
}

} // namespace osquery

