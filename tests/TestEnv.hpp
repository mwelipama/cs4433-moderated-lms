#pragma once
#include "dao/SqliteRepositories.hpp"
#include "domain/StandardPenaltyPolicy.hpp"
#include "infra/Argon2idPasswordHasher.hpp"
#include "infra/Clock.hpp"
#include "infra/Database.hpp"
#include "infra/SchemaInitializer.hpp"
#include "services/AssignmentService.hpp"
#include "services/AuditService.hpp"
#include "services/AuthService.hpp"
#include "services/CourseService.hpp"
#include "services/MarkingService.hpp"
#include "services/ModerationService.hpp"
#include "services/SubmissionService.hpp"
#include "services/UserAdminService.hpp"

#include <memory>
#include <string>

namespace lms {

// A controllable clock so time-dependent rules (deadlines, lateness) are deterministic.
class FixedClock : public Clock {
public:
    explicit FixedClock(const std::string& iso) : epoch_(DateTime::toEpoch(iso)) {}
    void set(const std::string& iso) { epoch_ = DateTime::toEpoch(iso); }
    std::string nowIso() const override { return DateTime::fromEpoch(epoch_); }
    int64_t nowEpoch() const override { return epoch_; }
private:
    int64_t epoch_;
};

// Fully wired stack over a fresh database, with a fast test hasher and an injectable clock.
// Mirrors the production composition root but with dependencies a test can steer.
struct TestEnv {
    Database db;
    SqliteUserRepository users;
    SqliteDepartmentRepository departments;
    SqliteCourseRepository courses;
    SqliteLecturerCourseRepository lecturerCourses;
    SqliteEnrolmentRepository enrolments;
    SqliteAssignmentRepository assignments;
    SqliteExtensionRepository extensions;
    SqliteSubmissionRepository submissions;
    SqliteMarkRepository marks;
    SqliteAuditRepository auditRepo;
    FixedClock clock;
    Argon2idPasswordHasher hasher;  // fast parameters for tests only
    StandardPenaltyPolicy policy;
    AuditService audit;
    AuthService auth;
    UserAdminService userAdmin;
    CourseService courseSvc;
    AssignmentService assignmentSvc;
    SubmissionService submissionSvc;
    MarkingService markingSvc;
    ModerationService moderationSvc;
    std::unique_ptr<User> admin;

    struct Ids { int64_t dept, course, lecturerId, hodId, studentId, assignmentId; };

    explicit TestEnv(const std::string& path = ":memory:",
                     const std::string& now = "2026-01-10T00:00:00")
        : db(path), users(db), departments(db), courses(db), lecturerCourses(db),
          enrolments(db), assignments(db), extensions(db), submissions(db), marks(db),
          auditRepo(db), clock(now), hasher(1, 256, 1), policy(),
          audit(auditRepo, clock), auth(users, hasher),
          userAdmin(users, departments, hasher),
          courseSvc(courses, departments, users, lecturerCourses, enrolments),
          assignmentSvc(assignments, courses, lecturerCourses, enrolments, extensions),
          submissionSvc(submissions, assignments, enrolments, extensions, marks, clock),
          markingSvc(submissions, marks, assignments, lecturerCourses, extensions,
                     audit, policy, db),
          moderationSvc(submissions, marks, assignments, courses, audit, db) {
        SchemaInitializer::ensureSchema(db);
    }

    std::unique_ptr<User> seedAdmin() {
        NewUser nu;
        nu.role = Role::Admin; nu.username = "root"; nu.displayName = "Root Admin";
        nu.passwordHash = hasher.hash("password1"); nu.active = true;
        return users.findById(users.create(nu));
    }

    std::unique_ptr<User> userById(int64_t id) { return users.findById(id); }

    // Creates a department, a course, an assigned lecturer, a department HoD, and one
    // enrolled student, plus an assignment. Default deadline and max mark suit most tests.
    Ids seedWorld(const std::string& deadline = "2026-02-01T00:00:00", int maxMark = 100) {
        admin = seedAdmin();
        Ids z{};
        z.dept = userAdmin.createDepartment(*admin, "Computer Science");
        z.lecturerId = userAdmin.createUser(*admin, Role::Lecturer, "lect", "password1", "Dr Lecturer");
        z.hodId = userAdmin.createUser(*admin, Role::Hod, "hod", "password1", "Prof Head", z.dept);
        z.studentId = userAdmin.createUser(*admin, Role::Student, "stud", "password1", "A Student");
        z.course = courseSvc.createCourse(*admin, "CS101", "Introduction", z.dept);
        courseSvc.assignLecturer(*admin, z.course, z.lecturerId);
        courseSvc.enrolStudent(*admin, z.course, z.studentId);
        auto lecturer = userById(z.lecturerId);
        z.assignmentId = assignmentSvc.createAssignment(*lecturer, z.course, "Assignment 1",
                                                        deadline, maxMark);
        return z;
    }

    int64_t addStudent(const std::string& username, int64_t courseId) {
        const int64_t id = userAdmin.createUser(*admin, Role::Student, username, "password1", username);
        courseSvc.enrolStudent(*admin, courseId, id);
        return id;
    }
};

} // namespace lms
