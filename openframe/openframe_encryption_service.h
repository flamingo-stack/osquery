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

#include <osquery/utils/status/status.h>

namespace osquery {

class OpenframeEncryptionService {
public:
    explicit OpenframeEncryptionService(const std::string& secret);
    ~OpenframeEncryptionService() = default;

    /**
     * Decrypts data using AES-GCM
     * @param data Base64 encoded encrypted data
     * @param decrypted Output parameter for the decrypted data
     * @return Status::success() on success, or an error Status describing
     *         the failure reason
     */
    Status decrypt(const std::string& data, std::string& decrypted);

    std::vector<unsigned char> base64Decode(const std::string& encoded);

private:
    static constexpr size_t kKeySize = 32; // 256 bits
    static constexpr size_t kIvSize = 12;  // 96 bits for GCM
    static constexpr size_t kTagSize = 16; // 128 bits for GCM

    void handleOpenSSLError();

    std::string secret_;
};

} // namespace osquery
