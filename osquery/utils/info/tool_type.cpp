/**
 * Copyright (c) 2014-present, The osquery authors
 *
 * This source code is licensed as defined by the LICENSE file found in the
 * root directory of this source tree.
 *
 * SPDX-License-Identifier: (Apache-2.0 OR GPL-2.0-only)
 */

#include <osquery/utils/info/tool_type.h>

namespace osquery {

namespace {

/// Current tool type.
ToolType gToolType{ToolType::UNKNOWN};

} // namespace

void setToolType(ToolType tool) {
  gToolType = tool;
}

ToolType getToolType() {
  return gToolType;
}

bool isDaemon() {
  return gToolType == ToolType::DAEMON;
}

bool isShell() {
  return gToolType == ToolType::SHELL;
}
} // namespace osquery

