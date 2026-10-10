/**
 *  Copyright (c) 2014-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under both the Apache 2.0 license (found in the
 *  LICENSE file in the root directory of this source tree) and the GPLv2 (found
 *  in the COPYING file in the root directory of this source tree).
 *  You may select, at your option, one of the above-listed licenses.
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
