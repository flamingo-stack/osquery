/**
 *  Copyright (c) 2014-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional
 *  grant of patent rights can be found in the PATENTS file in the same
 *  directory.
 */

#include "openframe_token_extractor.h"
#include <fstream>

#include <osquery/logger.h>

namespace osquery {

OpenframeTokenExtractor::OpenframeTokenExtractor(std::shared_ptr<OpenframeEncryptionService> encryption_service,
                                               const std::string& token_file_path)
    : encryption_service_(encryption_service), token_file_path_(token_file_path) {
    if (!encryption_service_) {
        LOG(ERROR) << "Encryption service cannot be null";
    }
    if (token_file_path_.empty()) {
        LOG(ERROR) << "Token file path cannot be empty";
    }
}

std::string OpenframeTokenExtractor::extractToken() {
    // Open the token file
    std::ifstream token_file(token_file_path_);
    if (!token_file.is_open()) {
        LOG(ERROR) << "Failed to open token file at: " << token_file_path_;
        return std::string();
    }

    // Read the encrypted token
    std::string encrypted_token;
    std::getline(token_file, encrypted_token);
    token_file.close();

    if (encrypted_token.empty()) {
        LOG(ERROR) << "Token file is empty";
        return std::string();
    }

    if (!encryption_service_) {
        LOG(ERROR) << "Encryption service is not available";
        return std::string();
    }

    // Decrypt the token using the encryption service
    return encryption_service_->decrypt(encrypted_token);
}

} // namespace osquery
