#include "infra/Argon2idPasswordHasher.hpp"

#include <argon2.h>

#include "infra/SecureRandom.hpp"

#include <array>
#include <stdexcept>
#include <vector>

namespace lms {

namespace {
constexpr size_t kSaltLen = 16;
constexpr size_t kHashLen = 32;

// 16 bytes from the operating system's cryptographically secure RNG for the salt.
std::array<unsigned char, kSaltLen> makeSalt() {
    std::array<unsigned char, kSaltLen> salt{};
    secureRandomBytes(salt.data(), salt.size());
    return salt;
}
} // namespace

Argon2idPasswordHasher::Argon2idPasswordHasher(uint32_t timeCost,
                                               uint32_t memoryCostKiB,
                                               uint32_t parallelism)
    : timeCost_(timeCost), memoryCostKiB_(memoryCostKiB), parallelism_(parallelism) {}

std::string Argon2idPasswordHasher::hash(const std::string& password) const {
    const auto salt = makeSalt();
    const size_t encodedLen = argon2_encodedlen(
        timeCost_, memoryCostKiB_, parallelism_,
        static_cast<uint32_t>(salt.size()), static_cast<uint32_t>(kHashLen), Argon2_id);
    std::vector<char> encoded(encodedLen, '\0');

    const int rc = argon2id_hash_encoded(
        timeCost_, memoryCostKiB_, parallelism_,
        password.data(), password.size(),
        salt.data(), salt.size(),
        kHashLen, encoded.data(), encoded.size());
    if (rc != ARGON2_OK)
        throw std::runtime_error(std::string("password hashing failed: ") +
                                 argon2_error_message(rc));
    return std::string(encoded.data());
}

bool Argon2idPasswordHasher::verify(const std::string& phc, const std::string& password) const {
    const int rc = argon2id_verify(phc.c_str(), password.data(), password.size());
    return rc == ARGON2_OK;
}

} // namespace lms
