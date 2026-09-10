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
#include <vector>
#include <memory>
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/err.h>
#include <stdexcept>

#include <osquery/utils/status/status.h>

namespace osquery {

class OpenframeEncryptionService {
public:
    explicit OpenframeEncryptionService(const std::string& secret);
    ~OpenframeEncryptionService() = default;

    /**
     * Decrypts data using AES-GCM
     * @param data Base64 encoded encrypted data
     * @param out Decrypted data as string, populated on success
     * @return Status::success() on success, Status::failure() with an error message on failure
     */
    Status decrypt(const std::string& data, std::string& out);

    std::vector<unsigned char> base64Decode(const std::string& encoded);

private:
    static constexpr size_t KEY_SIZE = 32; // 256 bits
    static constexpr size_t IV_SIZE = 12;  // 96 bits for GCM
    static constexpr size_t TAG_SIZE = 16; // 128 bits for GCM

    void handleOpenSSLError();

    std::string secret_;
};

} // namespace osquery
