#pragma once
#include "dao/Repositories.hpp"
#include "infra/PasswordHasher.hpp"
#include <memory>
#include <string>
#include <vector>

namespace lms {

// Administrator-only management of accounts and departments. Enforces the Administrator
// role, validates input, and hashes passwords before anything reaches the repository.
class UserAdminService {
public:
    UserAdminService(IUserRepository& users, IDepartmentRepository& departments,
                     PasswordHasher& hasher)
        : users_(users), departments_(departments), hasher_(hasher) {}

    int64_t createDepartment(const User& actor, const std::string& name);
    int64_t createUser(const User& actor, Role role, const std::string& username,
                       const std::string& password, const std::string& displayName,
                       int64_t departmentId = 0);  // departmentId used only for HOD
    void setUserActive(const User& actor, int64_t userId, bool active);
    std::vector<std::unique_ptr<User>> listUsers(const User& actor);
    std::vector<Department> listDepartments(const User& actor);

private:
    void requireAdmin(const User& actor) const;
    IUserRepository& users_;
    IDepartmentRepository& departments_;
    PasswordHasher& hasher_;
};

} // namespace lms
