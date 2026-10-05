#pragma once

#include <string>
#include <vector>
#include <memory>
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/err.h>
#include <stdexcept>

#include "osquery/core/status.h"

namespace osquery {

class OpenframeEncryptionService {
public:
    explicit OpenframeEncryptionService(const std::string& secret);
    ~OpenframeEncryptionService() = default;

    /**
     * Decrypts data using AES-GCM
     * @param data Base64 encoded encrypted data
     * @param decrypted Output parameter for decrypted data
     * @return Status indicating success or failure of decryption
     */
    Status decrypt(const std::string& data, std::string& decrypted);

    std::vector<unsigned char> base64Decode(const std::string& encoded);

private:
    static constexpr size_t KEY_SIZE = 32; // 256 bits
    static constexpr size_t IV_SIZE = 12;  // 96 bits for GCM
    static constexpr size_t TAG_SIZE = 16; // 128 bits for GCM

    void handleOpenSSLError();

    std::string secret_;
};

} // namespace osquery
