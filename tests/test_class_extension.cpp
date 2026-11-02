#include "doctest.h"
#include "TestEnv.hpp"
#include "domain/Errors.hpp"
#include "infra/Database.hpp"
#include <cstdio>

using namespace lms;

TEST_CASE("CR1: a class-wide extension makes an otherwise-late submission on time") {
    TestEnv env;
    auto ids = env.seedWorld("2026-02-01T00:00:00");
    auto lecturer = env.userById(ids.lecturerId);
    auto student = env.userById(ids.studentId);
    env.assignmentSvc.extendClass(*lecturer, ids.assignmentId, "2026-02-10T00:00:00");
    env.clock.set("2026-02-05T00:00:00");            // late vs base, on time vs class extension
    auto r = env.submissionSvc.submit(*student, ids.assignmentId, "work");
    CHECK(r.state == SubmissionState::Submitted);
}

TEST_CASE("CR1: only an authorised lecturer may extend a class deadline") {
    TestEnv env;
    auto ids = env.seedWorld();
    const int64_t otherLec =
        env.userAdmin.createUser(*env.admin, Role::Lecturer, "lec2", "password1", "Other Lecturer");
    auto other = env.userById(otherLec);
    CHECK_THROWS_AS(env.assignmentSvc.extendClass(*other, ids.assignmentId, "2026-02-10T00:00:00"),
                    AuthorizationError);
    auto student = env.userById(ids.studentId);
    CHECK_THROWS_AS(env.assignmentSvc.extendClass(*student, ids.assignmentId, "2026-02-10T00:00:00"),
                    AuthorizationError);
}

TEST_CASE("CR1: the most generous of base, class and individual extension applies") {
    TestEnv env;
    auto ids = env.seedWorld("2026-02-01T00:00:00");
    auto lecturer = env.userById(ids.lecturerId);
    auto assignment = env.assignments.findById(ids.assignmentId);

    env.assignmentSvc.extendClass(*lecturer, ids.assignmentId, "2026-02-10T00:00:00");
    CHECK(env.submissionSvc.effectiveDeadline(*assignment, ids.studentId) == "2026-02-10T00:00:00");

    // An individual extension EARLIER than the class one does not reduce the student's deadline.
    env.assignmentSvc.grantExtension(*lecturer, ids.assignmentId, ids.studentId, "2026-02-05T00:00:00");
    CHECK(env.submissionSvc.effectiveDeadline(*assignment, ids.studentId) == "2026-02-10T00:00:00");

    // An individual extension LATER than the class one wins.
    env.assignmentSvc.grantExtension(*lecturer, ids.assignmentId, ids.studentId, "2026-02-15T00:00:00");
    CHECK(env.submissionSvc.effectiveDeadline(*assignment, ids.studentId) == "2026-02-15T00:00:00");
}

TEST_CASE("CR1: a class extension flows into the late penalty at marking") {
    TestEnv env;
    auto ids = env.seedWorld("2026-02-01T00:00:00");
    auto lecturer = env.userById(ids.lecturerId);
    auto student = env.userById(ids.studentId);
    env.assignmentSvc.extendClass(*lecturer, ids.assignmentId, "2026-02-10T00:00:00");
    env.clock.set("2026-02-05T00:00:00");
    env.submissionSvc.submit(*student, ids.assignmentId, "work");
    const int64_t sid = env.submissions.find(ids.assignmentId, ids.studentId)->id;
    auto res = env.markingSvc.mark(*lecturer, sid, 80, "ok");
    CHECK(res.daysLate == 0);        // the class extension covered it
    CHECK(res.awardedMark == 80);    // so no penalty
}

TEST_CASE("CR1: without the extension the same submission is late (control)") {
    TestEnv env;
    auto ids = env.seedWorld("2026-02-01T00:00:00");
    auto student = env.userById(ids.studentId);
    env.clock.set("2026-02-02T00:00:00");            // 1 day late, no class extension
    auto r = env.submissionSvc.submit(*student, ids.assignmentId, "work");
    CHECK(r.state == SubmissionState::Late);
    CHECK(r.daysLate == 1);
}

TEST_CASE("CR1: a class extension persists across reopening the database") {
    const std::string path = "lms_test_classext.sqlite";
    std::remove(path.c_str());
    int64_t aid = 0;
    {
        TestEnv env(path);
        auto ids = env.seedWorld("2026-02-01T00:00:00");
        aid = ids.assignmentId;
        auto lecturer = env.userById(ids.lecturerId);
        env.assignmentSvc.extendClass(*lecturer, aid, "2026-02-10T00:00:00");
    }
    {
        Database db(path);
        SqliteClassExtensionRepository classExt(db);
        auto d = classExt.find(aid);
        REQUIRE(d.has_value());
        CHECK(*d == "2026-02-10T00:00:00");
    }
    std::remove(path.c_str());
}
