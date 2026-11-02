#pragma once
#include "domain/Enums.hpp"
#include <cstdint>
#include <string>

namespace lms {

// Abstract account. Holds the state common to every user and defers the role to the
// concrete subclass through the pure-virtual role(), giving the required polymorphic
// User hierarchy. Subclasses are substitutable wherever a User is expected (LSP).
class User {
public:
    User(int64_t id, std::string username, std::string passwordHash,
         std::string displayName, bool active)
        : id_(id), username_(std::move(username)), passwordHash_(std::move(passwordHash)),
          displayName_(std::move(displayName)), active_(active) {}
    virtual ~User() = default;

    virtual Role role() const = 0;

    int64_t id() const { return id_; }
    const std::string& username() const { return username_; }
    const std::string& passwordHash() const { return passwordHash_; }
    const std::string& displayName() const { return displayName_; }
    bool active() const { return active_; }

    // Only a Head of Department carries a department; the base reports "none".
    virtual bool hasDepartment() const { return false; }
    virtual int64_t departmentId() const { return 0; }

protected:
    int64_t id_;
    std::string username_;
    std::string passwordHash_;
    std::string displayName_;
    bool active_;
};

class Administrator : public User {
public:
    using User::User;
    Role role() const override { return Role::Admin; }
};

class Lecturer : public User {
public:
    using User::User;
    Role role() const override { return Role::Lecturer; }
};

class Student : public User {
public:
    using User::User;
    Role role() const override { return Role::Student; }
};

class HeadOfDepartment : public User {
public:
    HeadOfDepartment(int64_t id, std::string username, std::string passwordHash,
                     std::string displayName, bool active, int64_t departmentId)
        : User(id, std::move(username), std::move(passwordHash),
               std::move(displayName), active),
          departmentId_(departmentId) {}
    Role role() const override { return Role::Hod; }
    bool hasDepartment() const override { return true; }
    int64_t departmentId() const override { return departmentId_; }

private:
    int64_t departmentId_;
};

} // namespace lms
