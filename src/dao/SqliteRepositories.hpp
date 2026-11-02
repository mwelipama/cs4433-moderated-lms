#pragma once
#include "dao/Repositories.hpp"
#include "infra/Database.hpp"

namespace lms {

// SQLite-backed implementations. Each holds a reference to the shared Database connection
// and performs only parameterised statements; none open their own transactions, so a
// service can compose several repository calls inside one atomic unit of work.

class SqliteUserRepository : public IUserRepository {
public:
    explicit SqliteUserRepository(Database& db) : db_(db) {}
    int64_t create(const NewUser& u) override;
    std::unique_ptr<User> findById(int64_t id) override;
    std::unique_ptr<User> findByUsername(const std::string& username) override;
    bool usernameExists(const std::string& username) override;
    std::vector<std::unique_ptr<User>> list() override;
    void setActive(int64_t id, bool active) override;
private:
    Database& db_;
};

class SqliteDepartmentRepository : public IDepartmentRepository {
public:
    explicit SqliteDepartmentRepository(Database& db) : db_(db) {}
    int64_t create(const std::string& name) override;
    std::optional<Department> findById(int64_t id) override;
    std::optional<Department> findByName(const std::string& name) override;
    std::vector<Department> list() override;
private:
    Database& db_;
};

class SqliteCourseRepository : public ICourseRepository {
public:
    explicit SqliteCourseRepository(Database& db) : db_(db) {}
    int64_t create(const std::string& code, const std::string& title, int64_t deptId) override;
    std::optional<Course> findById(int64_t id) override;
    std::optional<Course> findByCode(const std::string& code) override;
    std::vector<Course> list() override;
private:
    Database& db_;
};

class SqliteLecturerCourseRepository : public ILecturerCourseRepository {
public:
    explicit SqliteLecturerCourseRepository(Database& db) : db_(db) {}
    void assign(int64_t courseId, int64_t lecturerId) override;
    bool isAssigned(int64_t courseId, int64_t lecturerId) override;
    std::vector<Course> coursesForLecturer(int64_t lecturerId) override;
private:
    Database& db_;
};

class SqliteEnrolmentRepository : public IEnrolmentRepository {
public:
    explicit SqliteEnrolmentRepository(Database& db) : db_(db) {}
    void enrol(int64_t courseId, int64_t studentId) override;
    bool isEnrolled(int64_t courseId, int64_t studentId) override;
    std::vector<Course> coursesForStudent(int64_t studentId) override;
    std::vector<std::unique_ptr<User>> studentsForCourse(int64_t courseId) override;
private:
    Database& db_;
};

class SqliteAssignmentRepository : public IAssignmentRepository {
public:
    explicit SqliteAssignmentRepository(Database& db) : db_(db) {}
    int64_t create(int64_t courseId, const std::string& title,
                   const std::string& deadline, int maxMark) override;
    std::optional<Assignment> findById(int64_t id) override;
    std::vector<Assignment> listByCourse(int64_t courseId) override;
private:
    Database& db_;
};

class SqliteExtensionRepository : public IExtensionRepository {
public:
    explicit SqliteExtensionRepository(Database& db) : db_(db) {}
    void set(int64_t assignmentId, int64_t studentId, const std::string& extendedDeadline) override;
    std::optional<Extension> find(int64_t assignmentId, int64_t studentId) override;
private:
    Database& db_;
};

class SqliteSubmissionRepository : public ISubmissionRepository {
public:
    explicit SqliteSubmissionRepository(Database& db) : db_(db) {}
    std::optional<Submission> find(int64_t assignmentId, int64_t studentId) override;
    std::optional<Submission> findById(int64_t id) override;
    int64_t insert(const Submission& s) override;
    void update(const Submission& s) override;
    void setState(int64_t id, SubmissionState state) override;
    std::vector<Submission> listByAssignment(int64_t assignmentId) override;
private:
    Database& db_;
};

class SqliteMarkRepository : public IMarkRepository {
public:
    explicit SqliteMarkRepository(Database& db) : db_(db) {}
    int64_t insert(int64_t submissionId, int rawMark, int awardedMark,
                   const std::string& feedback) override;
    void update(int64_t markId, int rawMark, int awardedMark,
                const std::string& feedback) override;
    std::optional<Mark> findBySubmission(int64_t submissionId) override;
    std::optional<Mark> findById(int64_t id) override;
private:
    Database& db_;
};

class SqliteAuditRepository : public IAuditRepository {
public:
    explicit SqliteAuditRepository(Database& db) : db_(db) {}
    int64_t append(const AuditEvent& e) override;
    std::vector<AuditEvent> listByMark(int64_t markId) override;
private:
    Database& db_;
};

} // namespace lms
