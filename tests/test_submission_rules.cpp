#include "doctest.h"
#include "TestEnv.hpp"
#include "domain/Errors.hpp"

using namespace lms;

TEST_CASE("Before the deadline a student may resubmit; the latest replaces the previous") {
    TestEnv env;
    auto ids = env.seedWorld("2026-02-01T00:00:00");
    auto student = env.userById(ids.studentId);

    env.clock.set("2026-01-20T09:00:00");
    auto r1 = env.submissionSvc.submit(*student, ids.assignmentId, "first draft");
    CHECK(r1.state == SubmissionState::Submitted);

    env.clock.set("2026-01-25T09:00:00");
    auto r2 = env.submissionSvc.submit(*student, ids.assignmentId, "second draft");
    CHECK(r2.state == SubmissionState::Submitted);

    auto current = env.submissions.find(ids.assignmentId, ids.studentId);
    REQUIRE(current.has_value());
    CHECK(current->content == "second draft");               // replaced, not duplicated
    CHECK(env.submissions.listByAssignment(ids.assignmentId).size() == 1);
}

TEST_CASE("A submission exactly at the deadline is on time") {
    TestEnv env;
    auto ids = env.seedWorld("2026-02-01T00:00:00");
    auto student = env.userById(ids.studentId);
    env.clock.set("2026-02-01T00:00:00");
    auto r = env.submissionSvc.submit(*student, ids.assignmentId, "right on the deadline");
    CHECK(r.state == SubmissionState::Submitted);
}

TEST_CASE("A first late submission is accepted within the 72-hour window") {
    TestEnv env;
    auto ids = env.seedWorld("2026-02-01T00:00:00");
    auto student = env.userById(ids.studentId);
    env.clock.set("2026-02-01T12:00:00");                     // 12 hours late
    auto r = env.submissionSvc.submit(*student, ids.assignmentId, "late work");
    CHECK(r.state == SubmissionState::Late);
    CHECK(r.daysLate == 1);
}

TEST_CASE("After the deadline an existing ON-TIME submission cannot be replaced") {
    TestEnv env;
    auto ids = env.seedWorld("2026-02-01T00:00:00");
    auto student = env.userById(ids.studentId);
    env.clock.set("2026-01-20T09:00:00");
    env.submissionSvc.submit(*student, ids.assignmentId, "on-time work");
    env.clock.set("2026-02-02T09:00:00");                     // now late window
    CHECK_THROWS_AS(env.submissionSvc.submit(*student, ids.assignmentId, "replacement"),
                    ConflictError);
    auto current = env.submissions.find(ids.assignmentId, ids.studentId);
    CHECK(current->content == "on-time work");                // preserved
    CHECK(current->state == SubmissionState::Submitted);
}

TEST_CASE("After the deadline an existing LATE submission also cannot be replaced") {
    TestEnv env;
    auto ids = env.seedWorld("2026-02-01T00:00:00");
    auto student = env.userById(ids.studentId);
    env.clock.set("2026-02-01T12:00:00");                     // first late submission
    env.submissionSvc.submit(*student, ids.assignmentId, "late work");
    env.clock.set("2026-02-02T12:00:00");                     // still within cutoff, tries again
    CHECK_THROWS_AS(env.submissionSvc.submit(*student, ids.assignmentId, "second late attempt"),
                    ConflictError);
    auto current = env.submissions.find(ids.assignmentId, ids.studentId);
    CHECK(current->content == "late work");                   // the first late stands
}

TEST_CASE("Nothing is accepted after the 72-hour cutoff when no submission exists") {
    TestEnv env;
    auto ids = env.seedWorld("2026-02-01T00:00:00");
    auto student = env.userById(ids.studentId);
    env.clock.set("2026-02-05T00:00:00");                     // > 72h after the deadline
    CHECK_THROWS_AS(env.submissionSvc.submit(*student, ids.assignmentId, "too late"),
                    ConflictError);
    CHECK_FALSE(env.submissions.find(ids.assignmentId, ids.studentId).has_value());
}

TEST_CASE("A student cannot submit to a course they are not enrolled in") {
    TestEnv env;
    auto ids = env.seedWorld();
    const int64_t outsiderId =
        env.userAdmin.createUser(*env.admin, Role::Student, "outsider", "password1", "Outsider");
    auto outsider = env.userById(outsiderId);
    env.clock.set("2026-01-20T00:00:00");
    CHECK_THROWS_AS(env.submissionSvc.submit(*outsider, ids.assignmentId, "x"), AuthorizationError);
}

TEST_CASE("An individual extension moves the effective deadline for that student") {
    TestEnv env;
    auto ids = env.seedWorld("2026-02-01T00:00:00");
    auto lecturer = env.userById(ids.lecturerId);
    auto student = env.userById(ids.studentId);
    env.assignmentSvc.grantExtension(*lecturer, ids.assignmentId, ids.studentId,
                                     "2026-02-10T00:00:00");
    env.clock.set("2026-02-05T00:00:00");                     // late vs base, on time vs extension
    auto r = env.submissionSvc.submit(*student, ids.assignmentId, "work within extension");
    CHECK(r.state == SubmissionState::Submitted);
}
