#include "services/UserAdminService.hpp"
#include "services/Validation.hpp"
#include "domain/Errors.hpp"

namespace lms {

namespace { constexpr size_t kMinPasswordLength = 8; }

void UserAdminService::requireAdmin(const User& actor) const {
    if (actor.role() != Role::Admin)
        throw AuthorizationError("only an administrator may perform this action");
}

int64_t UserAdminService::createDepartment(const User& actor, const std::string& name) {
    requireAdmin(actor);
    requireNonBlank(name, "department name");
    if (departments_.findByName(name))
        throw ConflictError("a department named '" + name + "' already exists");
    return departments_.create(trimWhitespace(name));
}

int64_t UserAdminService::createUser(const User& actor, Role role, const std::string& username,
                                     const std::string& password, const std::string& displayName,
                                     int64_t departmentId) {
    requireAdmin(actor);
    requireNonBlank(username, "username");
    requireNonBlank(displayName, "display name");
    if (password.size() < kMinPasswordLength)
        throw ValidationError("password must be at least " +
                              std::to_string(kMinPasswordLength) + " characters");
    const std::string uname = trimWhitespace(username);
    if (users_.usernameExists(uname))
        throw ConflictError("username '" + uname + "' is already taken");

    NewUser nu;
    nu.role = role;
    nu.username = uname;
    nu.passwordHash = hasher_.hash(password);
    nu.displayName = trimWhitespace(displayName);
    nu.active = true;
    if (role == Role::Hod) {
        if (!departments_.findById(departmentId))
            throw ValidationError("a Head of Department must be attached to an existing department");
        nu.hasDepartment = true;
        nu.departmentId = departmentId;
    }
    return users_.create(nu);
}

void UserAdminService::setUserActive(const User& actor, int64_t userId, bool active) {
    requireAdmin(actor);
    if (!users_.findById(userId))
        throw NotFoundError("no user with id " + std::to_string(userId));
    users_.setActive(userId, active);
}

std::vector<std::unique_ptr<User>> UserAdminService::listUsers(const User& actor) {
    requireAdmin(actor);
    return users_.list();
}

std::vector<Department> UserAdminService::listDepartments(const User& actor) {
    requireAdmin(actor);
    return departments_.list();
}

} // namespace lms
