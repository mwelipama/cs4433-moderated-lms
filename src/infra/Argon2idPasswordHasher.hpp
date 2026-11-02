#pragma once
#include "infra/PasswordHasher.hpp"
#include <cstdint>

namespace lms {

// Argon2id password hashing (NFR-03). The produced PHC string encodes the algorithm,
// version, cost parameters and a per-password random salt, so no salt or parameter is
// stored separately. Defaults follow current OWASP guidance for Argon2id.
class Argon2idPasswordHasher : public PasswordHasher {
public:
    Argon2idPasswordHasher(uint32_t timeCost = 3,
                           uint32_t memoryCostKiB = 19456,
                           uint32_t parallelism = 1);
    std::string hash(const std::string& password) const override;
    bool verify(const std::string& phc, const std::string& password) const override;

private:
    uint32_t timeCost_;
    uint32_t memoryCostKiB_;
    uint32_t parallelism_;
};

} // namespace lms
