#pragma once
#include "domain/Entities.hpp"
#include "domain/User.hpp"
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace lms {

// Fields needed to create a user account. passwordHash is already hashed by the service.
struct NewUser {
    Role role;
    std::string username;
    std::string passwordHash;
    std::string displayName;
    bool active{true};
    bool hasDepartment{false};
    int64_t departmentId{0};
};

// Each aggregate has its own narrow repository interface (Interface Segregation), so a
// service depends only on the persistence operations it actually uses. Concrete SQLite
// implementations are injected (Dependency Inversion), keeping business logic testable.

class IUserRepository {
public:
    virtual ~IUserRepository() = default;
    virtual int64_t create(const NewUser& u) = 0;
    virtual std::unique_ptr<User> findById(int64_t id) = 0;
    virtual std::unique_ptr<User> findByUsername(const std::string& username) = 0;
    virtual bool usernameExists(const std::string& username) = 0;
    virtual std::vector<std::unique_ptr<User>> list() = 0;
    virtual void setActive(int64_t id, bool active) = 0;
};

class IDepartmentRepository {
public:
    virtual ~IDepartmentRepository() = default;
    virtual int64_t create(const std::string& name) = 0;
    virtual std::optional<Department> findById(int64_t id) = 0;
    virtual std::optional<Department> findByName(const std::string& name) = 0;
    virtual std::vector<Department> list() = 0;
};

class ICourseRepository {
public:
    virtual ~ICourseRepository() = default;
    virtual int64_t create(const std::string& code, const std::string& title, int64_t deptId) = 0;
    virtual std::optional<Course> findById(int64_t id) = 0;
    virtual std::optional<Course> findByCode(const std::string& code) = 0;
    virtual std::vector<Course> list() = 0;
};

class ILecturerCourseRepository {
public:
    virtual ~ILecturerCourseRepository() = default;
    virtual void assign(int64_t courseId, int64_t lecturerId) = 0;
    virtual bool isAssigned(int64_t courseId, int64_t lecturerId) = 0;
    virtual std::vector<Course> coursesForLecturer(int64_t lecturerId) = 0;
};

class IEnrolmentRepository {
public:
    virtual ~IEnrolmentRepository() = default;
    virtual void enrol(int64_t courseId, int64_t studentId) = 0;
    virtual bool isEnrolled(int64_t courseId, int64_t studentId) = 0;
    virtual std::vector<Course> coursesForStudent(int64_t studentId) = 0;
    virtual std::vector<std::unique_ptr<User>> studentsForCourse(int64_t courseId) = 0;
};

class IAssignmentRepository {
public:
    virtual ~IAssignmentRepository() = default;
    virtual int64_t create(int64_t courseId, const std::string& title,
                           const std::string& deadline, int maxMark) = 0;
    virtual std::optional<Assignment> findById(int64_t id) = 0;
    virtual std::vector<Assignment> listByCourse(int64_t courseId) = 0;
};

class IExtensionRepository {
public:
    virtual ~IExtensionRepository() = default;
    virtual void set(int64_t assignmentId, int64_t studentId,
                     const std::string& extendedDeadline) = 0;  // insert or replace
    virtual std::optional<Extension> find(int64_t assignmentId, int64_t studentId) = 0;
};

class IClassExtensionRepository {
public:
    virtual ~IClassExtensionRepository() = default;
    virtual void set(int64_t assignmentId, const std::string& extendedDeadline) = 0;  // insert or replace
    virtual std::optional<std::string> find(int64_t assignmentId) = 0;                // the class deadline, if any
};

class ISubmissionRepository {
public:
    virtual ~ISubmissionRepository() = default;
    virtual std::optional<Submission> find(int64_t assignmentId, int64_t studentId) = 0;
    virtual std::optional<Submission> findById(int64_t id) = 0;
    virtual int64_t insert(const Submission& s) = 0;
    virtual void update(const Submission& s) = 0;             // content, submittedAt, state by id
    virtual void setState(int64_t id, SubmissionState state) = 0;
    virtual std::vector<Submission> listByAssignment(int64_t assignmentId) = 0;
};

class IMarkRepository {
public:
    virtual ~IMarkRepository() = default;
    virtual int64_t insert(int64_t submissionId, int rawMark, int awardedMark,
                           const std::string& feedback) = 0;
    virtual void update(int64_t markId, int rawMark, int awardedMark,
                        const std::string& feedback) = 0;
    virtual std::optional<Mark> findBySubmission(int64_t submissionId) = 0;
    virtual std::optional<Mark> findById(int64_t id) = 0;
};

class IAuditRepository {
public:
    virtual ~IAuditRepository() = default;
    virtual int64_t append(const AuditEvent& e) = 0;
    virtual std::vector<AuditEvent> listByMark(int64_t markId) = 0;
};

} // namespace lms
