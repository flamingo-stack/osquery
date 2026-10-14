/**
 * Copyright (c) 2014-present, The osquery authors
 *
 * This source code is licensed as defined by the LICENSE file found in the
 * root directory of this source tree.
 *
 * SPDX-License-Identifier: (Apache-2.0 OR GPL-2.0-only)
 */

#include <cstdlib>
#include <fstream>

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>

#include <osquery/core/tables.h>
#include <osquery/filesystem/filesystem.h>
#include <osquery/logger/logger.h>

namespace osquery {
namespace tables {

const std::string kLinuxArpTable = "/proc/net/arp";

// ARP flag bits as defined by the kernel (see <linux/if_arp.h>).
static const unsigned long kAtfCom = 0x02;
static const unsigned long kAtfPerm = 0x04;

QueryData genArpCache(QueryContext& context) {
  QueryData results;

  boost::filesystem::path arp_path = kLinuxArpTable;
  if (!osquery::isReadable(arp_path).ok()) {
    VLOG(1) << "Cannot read arp table";
    return results;
  }

  std::ifstream fd(arp_path.string(), std::ios::in | std::ios::binary);
  std::string line;

  if (fd.fail() || fd.eof()) {
    VLOG(1) << "Empty or failed arp table";
    return results;
  }

  // Read the header line.
  std::getline(fd, line, '\n');
  while (!(fd.fail() || fd.eof())) {
    std::getline(fd, line, '\n');

    // IP address, HW type, Flags, HW address, Mask Device
    std::vector<std::string> fields;
    boost::split(fields, line, boost::is_any_of(" "), boost::token_compress_on);
    for (auto& f : fields) {
      // Inline trim each split.
      boost::trim(f);
    }

    if (fields.size() != 6) {
      // An unhandled error case.
      continue;
    }

    Row r;
    r["address"] = fields[0];
    r["mac"] = fields[3];
    r["interface"] = fields[5];

    // Note: it's also possible to detect publish entries (ATF_PUB).
    unsigned long flags = strtoul(fields[2].c_str(), nullptr, 16);
    if ((flags & (kAtfCom | kAtfPerm)) == (kAtfCom | kAtfPerm)) {
      r["permanent"] = "1";
    } else {
      r["permanent"] = "0";
    }

    results.push_back(r);
  }

  return results;
}
}
}
