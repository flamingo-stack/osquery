/**
 * Copyright (c) 2014-present, The osquery authors
 *
 * This source code is licensed as defined by the LICENSE file found in the
 * root directory of this source tree.
 *
 * SPDX-License-Identifier: (Apache-2.0 OR GPL-2.0-only)
 */

#include "openframe_token_extractor.h"
#include <fstream>

#include <osquery/logger/logger.h>

namespace osquery {

OpenframeTokenExtractor::OpenframeTokenExtractor(std::shared_ptr<OpenframeEncryptionService> encryption_service,
                                               const std::string& token_file_path)
    : encryption_service_(encryption_service), token_file_path_(token_file_path) {
    if (!encryption_service_) {
        LOG(ERROR) << "Encryption service cannot be null";
        throw std::runtime_error("Encryption service cannot be null");
    }
    if (token_file_path_.empty()) {
        LOG(ERROR) << "Token file path cannot be empty";
        throw std::runtime_error("Token file path cannot be empty");
    }
}

Status OpenframeTokenExtractor::extractToken(std::string& token) {
    // Open the token file
    std::ifstream token_file(token_file_path_);
    if (!token_file.is_open()) {
        LOG(WARNING) << "Failed to open token file at: " << token_file_path_;
        return Status::failure("Failed to open token file at: " + token_file_path_);
    }

    // Read the encrypted token
    std::string encrypted_token;
    std::getline(token_file, encrypted_token);
    token_file.close();

    if (encrypted_token.empty()) {
        LOG(WARNING) << "Token file is empty: " << token_file_path_;
        return Status::failure("Token file is empty");
    }

    try {
        // Decrypt the token using the encryption service
        token = encryption_service_->decrypt(encrypted_token);
        return Status::success();
    } catch (const std::exception& e) {
        LOG(WARNING) << "Failed to decrypt token: " << e.what();
        return Status::failure("Failed to decrypt token: " + std::string(e.what()));
    }
}

} // namespace osquery
