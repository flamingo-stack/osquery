#include "openframe_encryption_service.h"
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>
#include <sstream>
#include <iomanip>

namespace {
constexpr size_t kAes256KeySize = 32;
}

OpenframeEncryptionService::OpenframeEncryptionService(const std::string& secret)
    : secret_(secret), initStatus_(osquery::Status::success()) {
    if (secret_.empty()) {
        initStatus_ = osquery::Status(1, "Secret cannot be empty");
    } else if (secret_.size() != kAes256KeySize) {
        initStatus_ = osquery::Status(1, "Secret must be exactly 32 bytes for AES-256-GCM");
    }
}

osquery::Status OpenframeEncryptionService::decrypt(const std::string& data, std::string& result) {
    if (!initStatus_.ok()) {
        return initStatus_;
    }

    if (secret_.empty()) {
        return osquery::Status(1, "Encryption service not initialized with secret");
    }

    if (secret_.size() != kAes256KeySize) {
        return osquery::Status(1, "Secret must be exactly 32 bytes for AES-256-GCM");
    }

    // Decode base64 data
    std::vector<unsigned char> decoded;
    osquery::Status decodeStatus = base64Decode(data, decoded);
    if (!decodeStatus.ok()) {
        return decodeStatus;
    }

    if (decoded.size() < IV_SIZE + TAG_SIZE) {
        return osquery::Status(1, "Invalid encrypted data size");
    }

    // Extract IV (first 12 bytes) and tag (last 16 bytes)
    std::vector<unsigned char> iv(decoded.begin(), decoded.begin() + IV_SIZE);
    std::vector<unsigned char> tag(decoded.end() - TAG_SIZE, decoded.end());
    std::vector<unsigned char> ciphertext(decoded.begin() + IV_SIZE, decoded.end() - TAG_SIZE);

    // Create and initialize the context
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        return getOpenSSLErrorStatus();
    }

    // Initialize the decryption operation
    if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, 
                               reinterpret_cast<const unsigned char*>(secret_.c_str()), 
                               iv.data())) {
        osquery::Status status = getOpenSSLErrorStatus();
        EVP_CIPHER_CTX_free(ctx);
        return status;
    }

    // Set the tag
    if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, TAG_SIZE, tag.data())) {
        osquery::Status status = getOpenSSLErrorStatus();
        EVP_CIPHER_CTX_free(ctx);
        return status;
    }

    // Decrypt the ciphertext
    std::vector<unsigned char> plaintext(ciphertext.size());
    int len = 0;
    if (1 != EVP_DecryptUpdate(ctx, plaintext.data(), &len, 
                              ciphertext.data(), ciphertext.size())) {
        osquery::Status status = getOpenSSLErrorStatus();
        EVP_CIPHER_CTX_free(ctx);
        return status;
    }

    // Finalize the decryption
    int finalLen = 0;
    if (1 != EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &finalLen)) {
        osquery::Status status = getOpenSSLErrorStatus();
        EVP_CIPHER_CTX_free(ctx);
        return status;
    }

    // Clean up
    EVP_CIPHER_CTX_free(ctx);

    // Convert the decrypted data to string
    result = std::string(plaintext.begin(), plaintext.begin() + len + finalLen);
    return osquery::Status::success();
}

osquery::Status OpenframeEncryptionService::base64Decode(const std::string& encoded,
                                                          std::vector<unsigned char>& decodedOut) {
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    
    BIO* bmem = BIO_new_mem_buf(encoded.c_str(), encoded.length());
    bmem = BIO_push(b64, bmem);

    std::vector<unsigned char> decoded(encoded.length());
    int decodedLen = BIO_read(bmem, decoded.data(), encoded.length());

    BIO_free_all(bmem);

    if (decodedLen < 0) {
        return osquery::Status(1, "Failed to decode base64 data");
    }

    decoded.resize(decodedLen);
    decodedOut = std::move(decoded);
    return osquery::Status::success();
}

osquery::Status OpenframeEncryptionService::getOpenSSLErrorStatus() {
    std::stringstream ss;
    unsigned long err;
    while ((err = ERR_get_error()) != 0) {
        char err_buf[256];
        ERR_error_string_n(err, err_buf, sizeof(err_buf));
        ss << err_buf << "; ";
    }
    return osquery::Status(1, "OpenSSL error: " + ss.str());
} 
