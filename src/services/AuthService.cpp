#include "services/AuthService.hpp"
#include "domain/Errors.hpp"

namespace lms {

std::unique_ptr<User> AuthService::login(const std::string& username, const std::string& password) {
    // One message for "no such user" and "wrong password" so accounts cannot be enumerated.
    auto user = users_.findByUsername(username);
    if (!user) throw AuthenticationError("invalid username or password");
    if (!user->active()) throw AuthenticationError("this account is inactive");
    if (!hasher_.verify(user->passwordHash(), password))
        throw AuthenticationError("invalid username or password");
    return user;
}

} // namespace lms
