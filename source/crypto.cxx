#include "headers/crypto.hxx"

std::vector<uint8_t> Crypto::Sha256Digest(const std::vector<uint8_t>& bytes) {
    std::vector<uint8_t> sha256_digest(SHA256_DIGEST_LENGTH);

    SHA256_CTX ossl_sha256;
    SHA256_Init(&ossl_sha256);
    SHA256_Update(&ossl_sha256, bytes.data(), bytes.size());
    SHA256_Final(sha256_digest.data(), &ossl_sha256);

    return sha256_digest;
}

std::vector<uint8_t> Crypto::ApplyPkcs7(std::vector<uint8_t> bytes, const uint8_t& multiple) {
    uint8_t difference = multiple - (bytes.size() % multiple);
    if(difference == multiple) return bytes;
    bytes.resize(bytes.size() + difference, difference);
    return bytes;
}

std::vector<uint8_t> Crypto::StripPkcs7(std::vector<uint8_t> bytes) {
    const uint8_t& last_byte = bytes.at(bytes.size()-1);

    const auto& validator_lambda = [&last_byte](const uint8_t& value) {
        return value == last_byte;
    };

    if(last_byte <= bytes.size() && std::all_of(bytes.cend()-last_byte, bytes.cend(), validator_lambda)) {
        bytes.resize(bytes.size()-last_byte);
    }

    return bytes;
}

std::vector<uint8_t> Crypto::Aes256CbcAutoEncrypt(std::vector<uint8_t> bytes, const std::string& plain_key) {
    const std::vector<uint8_t>& digested_key = Sha256Digest(std::vector<uint8_t>(plain_key.begin(), plain_key.end()));
    std::vector<uint8_t> initialization_vector(digested_key.begin(), digested_key.begin()+AES_BLOCK_SIZE);

    bytes = ApplyPkcs7(bytes, AES_BLOCK_SIZE);

    AES_KEY key_object;

    AES_set_encrypt_key(digested_key.data(), 256, &key_object);

    AES_cbc_encrypt(
        bytes.data(),
        bytes.data(),
        bytes.size(),
        &key_object,
        initialization_vector.data(),
        AES_ENCRYPT
    );

    return bytes;
}

std::vector<uint8_t> Crypto::Aes256CbcAutoDecrypt(std::vector<uint8_t> bytes, const std::string& plain_key) {
    const std::vector<uint8_t>& digested_key = Sha256Digest(std::vector<uint8_t>(plain_key.begin(), plain_key.end()));
    std::vector<uint8_t> initialization_vector(digested_key.begin(), digested_key.begin()+AES_BLOCK_SIZE);

    AES_KEY key_object;

    AES_set_decrypt_key(digested_key.data(), 256, &key_object);

    AES_cbc_encrypt(
        bytes.data(),
        bytes.data(),
        bytes.size(),
        &key_object,
        initialization_vector.data(),
        AES_DECRYPT
    );

    return StripPkcs7(bytes);
}