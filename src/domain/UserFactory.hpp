#pragma once
#include "domain/User.hpp"
#include <memory>
#include <string>

namespace lms {

// Centralises the role -> subclass mapping. Account creation and row hydration both go
// through it, so nothing else in the system switches on role to build a user, and the
// polymorphic User type is preserved everywhere.
class UserFactory {
public:
    static std::unique_ptr<User> create(Role role, int64_t id, const std::string& username,
                                        const std::string& passwordHash,
                                        const std::string& displayName, bool active,
                                        bool hasDepartment = false, int64_t departmentId = 0);
};

} // namespace lms
