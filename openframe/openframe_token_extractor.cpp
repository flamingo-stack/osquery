/**
 *  Copyright (c) 2014-present, The osquery authors
 *
 * This source code is licensed as defined by the LICENSE file found in the
 * root directory of this source tree.
 *
 * SPDX-License-Identifier: (Apache-2.0 OR GPL-2.0-only)
 */

#include "openframe_token_extractor.h"
#include <fstream>

namespace osquery {

OpenframeTokenExtractor::OpenframeTokenExtractor(std::shared_ptr<OpenframeEncryptionService> encryption_service,
                                               const std::string& token_file_path)
    : encryption_service_(encryption_service), token_file_path_(token_file_path) {
}

Status OpenframeTokenExtractor::extractToken(std::string& token) {
    if (!encryption_service_) {
        return Status::failure("Encryption service cannot be null");
    }
    if (token_file_path_.empty()) {
        return Status::failure("Token file path cannot be empty");
    }

    // Open the token file
    std::ifstream token_file(token_file_path_);
    if (!token_file.is_open()) {
        return Status::failure("Failed to open token file at: " + token_file_path_);
    }

    // Read the encrypted token
    std::string encrypted_token;
    std::getline(token_file, encrypted_token);
    token_file.close();

    if (encrypted_token.empty()) {
        return Status::failure("Token file is empty");
    }

    try {
        // Decrypt the token using the encryption service
        token = encryption_service_->decrypt(encrypted_token);
    } catch (const std::exception& e) {
        return Status::failure("Failed to decrypt token: " + std::string(e.what()));
    }

    return Status::success();
}

} // namespace osquery
