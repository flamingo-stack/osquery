/**
 * Copyright (c) 2014-present, The osquery authors
 *
 * This source code is licensed as defined by the LICENSE file found in the
 * root directory of this source tree.
 *
 * SPDX-License-Identifier: (Apache-2.0 OR GPL-2.0-only)
 */

#include "openframe_encryption_service.h"
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>
#include <sstream>
#include <iomanip>

#include <osquery/logger/logger.h>

namespace osquery {

OpenframeEncryptionService::OpenframeEncryptionService(const std::string& secret)
    : secret_(secret) {
}

Status OpenframeEncryptionService::decrypt(const std::string& data, std::string& result) {
    if (secret_.empty()) {
        return Status::failure("Encryption service not initialized with secret");
    }

    if (secret_.size() != 32) {
        return Status::failure("Secret must be exactly 32 bytes for AES-256-GCM");
    }

    // Decode base64 data
    std::vector<unsigned char> decoded;
    auto status = base64Decode(data, decoded);
    if (!status.ok()) {
        return status;
    }
    if (decoded.size() < IV_SIZE + TAG_SIZE) {
        return Status::failure("Invalid encrypted data size");
    }

    // Extract IV (first 12 bytes) and tag (last 16 bytes)
    std::vector<unsigned char> iv(decoded.begin(), decoded.begin() + IV_SIZE);
    std::vector<unsigned char> tag(decoded.end() - TAG_SIZE, decoded.end());
    std::vector<unsigned char> ciphertext(decoded.begin() + IV_SIZE, decoded.end() - TAG_SIZE);

    // Create and initialize the context
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        return handleOpenSSLError();
    }

    // Initialize the decryption operation
    if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, 
                               reinterpret_cast<const unsigned char*>(secret_.c_str()), 
                               iv.data())) {
        EVP_CIPHER_CTX_free(ctx);
        return handleOpenSSLError();
    }

    // Set the tag
    if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, TAG_SIZE, tag.data())) {
        EVP_CIPHER_CTX_free(ctx);
        return handleOpenSSLError();
    }

    // Decrypt the ciphertext
    std::vector<unsigned char> plaintext(ciphertext.size());
    int len = 0;
    if (1 != EVP_DecryptUpdate(ctx, plaintext.data(), &len, 
                              ciphertext.data(), ciphertext.size())) {
        EVP_CIPHER_CTX_free(ctx);
        return handleOpenSSLError();
    }

    // Finalize the decryption
    int finalLen = 0;
    if (1 != EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &finalLen)) {
        EVP_CIPHER_CTX_free(ctx);
        return handleOpenSSLError();
    }

    // Clean up
    EVP_CIPHER_CTX_free(ctx);

    // Convert the decrypted data to string
    result = std::string(plaintext.begin(), plaintext.begin() + len + finalLen);
    return Status::success();
}

Status OpenframeEncryptionService::base64Decode(const std::string& encoded, std::vector<unsigned char>& result) {
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    
    BIO* bmem = BIO_new_mem_buf(encoded.c_str(), encoded.length());
    bmem = BIO_push(b64, bmem);

    std::vector<unsigned char> decoded(encoded.length());
    int decodedLen = BIO_read(bmem, decoded.data(), encoded.length());

    BIO_free_all(bmem);

    if (decodedLen < 0) {
        return Status::failure("Failed to decode base64 data");
    }

    decoded.resize(decodedLen);
    result = std::move(decoded);
    return Status::success();
}

Status OpenframeEncryptionService::handleOpenSSLError() {
    std::stringstream ss;
    unsigned long err;
    while ((err = ERR_get_error()) != 0) {
        char err_buf[256];
        ERR_error_string_n(err, err_buf, sizeof(err_buf));
        ss << err_buf << "; ";
    }
    std::string message = "OpenSSL error: " + ss.str();
    LOG(ERROR) << message;
    return Status::failure(message);
}

} // namespace osquery
