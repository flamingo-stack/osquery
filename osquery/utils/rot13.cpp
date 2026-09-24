/**
 * Copyright (c) 2014-present, The osquery authors
 *
 * This source code is licensed as defined by the LICENSE file found in the
 * root directory of this source tree.
 *
 * SPDX-License-Identifier: (Apache-2.0 OR GPL-2.0-only)
 */

#include <osquery/utils/rot13.h>

#include <cctype>
#include <string>

namespace osquery {

std::string rotDecode(const std::string& rot_string) {
  std::string decoded_string;

  for (std::size_t i = 0; i < rot_string.size(); i++) {
    unsigned char c = static_cast<unsigned char>(rot_string[i]);
    if (isalpha(c)) {
      if (c >= 'a' && c <= 'z') {
        if (c <= 'm') {
          decoded_string.append(1, static_cast<char>(c + 13));
        } else {
          decoded_string.append(1, static_cast<char>(c - 13));
        }
      } else if (c >= 'A' && c <= 'Z') {
        if (c <= 'M') {
          decoded_string.append(1, static_cast<char>(c + 13));
        } else {
          decoded_string.append(1, static_cast<char>(c - 13));
        }
      }
    } else {
      decoded_string.append(1, rot_string[i]);
    }
  }
  return decoded_string;
}
} // namespace osquery
