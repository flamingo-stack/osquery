/**
 * Copyright (c) 2014-present, The osquery authors
 *
 * This source code is licensed as defined by the LICENSE file found in the
 * root directory of this source tree.
 *
 * SPDX-License-Identifier: (Apache-2.0 OR GPL-2.0-only)
 */

#pragma once

#include <string>
#include <memory>
#include "openframe_encryption_service.h"

namespace osquery {

class OpenframeTokenExtractor {
public:
    explicit OpenframeTokenExtractor(std::shared_ptr<OpenframeEncryptionService> encryption_service,
                                   const std::string& token_file_path);
    ~OpenframeTokenExtractor() = default;

    // Extract and decrypt the token from the file
    std::string extractToken();

private:
    std::string token_file_path_;
    std::shared_ptr<OpenframeEncryptionService> encryption_service_;
};

} // namespace osquery
