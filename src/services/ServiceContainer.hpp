#pragma once
#include "dao/SqliteRepositories.hpp"
#include "domain/StandardPenaltyPolicy.hpp"
#include "infra/Argon2idPasswordHasher.hpp"
#include "infra/Clock.hpp"
#include "infra/Database.hpp"
#include "services/AssignmentService.hpp"
#include "services/AuditService.hpp"
#include "services/AuthService.hpp"
#include "services/CourseService.hpp"
#include "services/MarkingService.hpp"
#include "services/ModerationService.hpp"
#include "services/SubmissionService.hpp"
#include "services/UserAdminService.hpp"

namespace lms {

// Composition root. Owns the repositories, strategies and services and wires them to a
// single Database connection. Declaration order matters: a member may only reference
// members declared before it, so repositories and strategies precede the services.
class ServiceContainer {
public:
    explicit ServiceContainer(Database& db)
        : db_(db),
          users_(db), departments_(db), courses_(db), lecturerCourses_(db), enrolments_(db),
          assignments_(db), extensions_(db), submissions_(db), marks_(db), auditRepo_(db),
          clock_(), hasher_(), policy_(),
          audit_(auditRepo_, clock_),
          auth_(users_, hasher_),
          userAdmin_(users_, departments_, hasher_),
          courseSvc_(courses_, departments_, users_, lecturerCourses_, enrolments_),
          assignmentSvc_(assignments_, courses_, lecturerCourses_, enrolments_, extensions_),
          submissionSvc_(submissions_, assignments_, enrolments_, extensions_, marks_, clock_),
          markingSvc_(submissions_, marks_, assignments_, lecturerCourses_, extensions_,
                      audit_, policy_, db_),
          moderationSvc_(submissions_, marks_, assignments_, courses_, audit_, db_) {}

    // First-run bootstrap: the only path that creates a user without an acting admin,
    // used once on an empty database to seed the initial administrator.
    bool hasAnyUser() { return !users_.list().empty(); }
    int64_t createInitialAdmin(const std::string& username, const std::string& password,
                               const std::string& displayName) {
        NewUser nu;
        nu.role = Role::Admin;
        nu.username = username;
        nu.passwordHash = hasher_.hash(password);
        nu.displayName = displayName;
        nu.active = true;
        return users_.create(nu);
    }

    // Services (the write side and guarded reads).
    AuthService& auth() { return auth_; }
    UserAdminService& userAdmin() { return userAdmin_; }
    CourseService& courses() { return courseSvc_; }
    AssignmentService& assignments() { return assignmentSvc_; }
    SubmissionService& submissions() { return submissionSvc_; }
    MarkingService& marking() { return markingSvc_; }
    ModerationService& moderation() { return moderationSvc_; }

    // Repositories (used by the console for plain, non-sensitive listings).
    IUserRepository& userRepo() { return users_; }
    IDepartmentRepository& departmentRepo() { return departments_; }
    ICourseRepository& courseRepo() { return courses_; }
    ILecturerCourseRepository& lecturerCourseRepo() { return lecturerCourses_; }
    IEnrolmentRepository& enrolmentRepo() { return enrolments_; }
    IAssignmentRepository& assignmentRepo() { return assignments_; }
    ISubmissionRepository& submissionRepo() { return submissions_; }
    IMarkRepository& markRepo() { return marks_; }
    IAuditRepository& auditRepo() { return auditRepo_; }

private:
    Database& db_;

    SqliteUserRepository users_;
    SqliteDepartmentRepository departments_;
    SqliteCourseRepository courses_;
    SqliteLecturerCourseRepository lecturerCourses_;
    SqliteEnrolmentRepository enrolments_;
    SqliteAssignmentRepository assignments_;
    SqliteExtensionRepository extensions_;
    SqliteSubmissionRepository submissions_;
    SqliteMarkRepository marks_;
    SqliteAuditRepository auditRepo_;

    SystemClock clock_;
    Argon2idPasswordHasher hasher_;
    StandardPenaltyPolicy policy_;

    AuditService audit_;
    AuthService auth_;
    UserAdminService userAdmin_;
    CourseService courseSvc_;
    AssignmentService assignmentSvc_;
    SubmissionService submissionSvc_;
    MarkingService markingSvc_;
    ModerationService moderationSvc_;
};

} // namespace lms
