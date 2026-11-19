/**
 * Copyright (c) 2014-present, The osquery authors
 *
 * This source code is licensed as defined by the LICENSE file found in the
 * root directory of this source tree.
 *
 * SPDX-License-Identifier: (Apache-2.0 OR GPL-2.0-only)
 */
#pragma once

#include <map>
#include <string>

#include <osquery/core/tables.h>

namespace osquery {
namespace tables {

/**
 * @brief Build the os_version row from the Windows NT CurrentVersion key
 *
 * @param currentVersion Value name to data of
 * HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion
 * @param arch The OS architecture as Win32_OperatingSystem reports it
 * @return One row, or none if the key lacks the product name or build
 */
QueryData genOSVersionFromRegistry(
    const std::map<std::string, std::string>& currentVersion,
    const std::string& arch);

} // namespace tables
} // namespace osquery
