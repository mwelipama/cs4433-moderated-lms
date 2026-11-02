#include "domain/UserFactory.hpp"
#include "domain/Errors.hpp"

namespace lms {

std::unique_ptr<User> UserFactory::create(Role role, int64_t id, const std::string& username,
                                          const std::string& passwordHash,
                                          const std::string& displayName, bool active,
                                          bool hasDepartment, int64_t departmentId) {
    switch (role) {
        case Role::Admin:
            return std::make_unique<Administrator>(id, username, passwordHash, displayName, active);
        case Role::Lecturer:
            return std::make_unique<Lecturer>(id, username, passwordHash, displayName, active);
        case Role::Student:
            return std::make_unique<Student>(id, username, passwordHash, displayName, active);
        case Role::Hod:
            if (!hasDepartment)
                throw ValidationError("a Head of Department requires a department");
            return std::make_unique<HeadOfDepartment>(id, username, passwordHash,
                                                       displayName, active, departmentId);
    }
    throw ValidationError("unknown role in UserFactory");
}

} // namespace lms
