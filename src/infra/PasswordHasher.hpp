#pragma once
#include <string>

namespace lms {

// Password hashing behind an interface so the algorithm is an injected dependency
// (Dependency Inversion) and can be swapped without touching the services that use it.
class PasswordHasher {
public:
    virtual ~PasswordHasher() = default;
    // Returns a self-describing PHC string that embeds the algorithm, parameters and salt.
    virtual std::string hash(const std::string& password) const = 0;
    // Constant-time verification of a password against a stored PHC string.
    virtual bool verify(const std::string& phc, const std::string& password) const = 0;
};

} // namespace lms
