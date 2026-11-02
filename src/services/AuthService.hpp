#pragma once
#include "dao/Repositories.hpp"
#include "infra/PasswordHasher.hpp"
#include <memory>
#include <string>

namespace lms {

// Authenticates a username/password against the stored Argon2id hash. Returns the
// authenticated User (polymorphic) or throws AuthenticationError.
class AuthService {
public:
    AuthService(IUserRepository& users, PasswordHasher& hasher)
        : users_(users), hasher_(hasher) {}

    std::unique_ptr<User> login(const std::string& username, const std::string& password);

private:
    IUserRepository& users_;
    PasswordHasher& hasher_;
};

} // namespace lms
