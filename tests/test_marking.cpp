#include "doctest.h"
#include "TestEnv.hpp"
#include "domain/Errors.hpp"

using namespace lms;

namespace {
// Submit on time as the seeded student and return the submission id.
int64_t onTimeSubmission(TestEnv& env, const TestEnv::Ids& ids) {
    auto student = env.userById(ids.studentId);
    env.clock.set("2026-01-20T00:00:00");
    env.submissionSvc.submit(*student, ids.assignmentId, "work");
    return env.submissions.find(ids.assignmentId, ids.studentId)->id;
}
} // namespace

TEST_CASE("Initial marking stores the mark, moves state to Marked, and writes an INITIAL audit") {
    TestEnv env;
    auto ids = env.seedWorld();
    const int64_t sid = onTimeSubmission(env, ids);
    auto lecturer = env.userById(ids.lecturerId);

    auto result = env.markingSvc.mark(*lecturer, sid, 80, "good work");
    CHECK(result.awardedMark == 80);                           // on time: no penalty

    auto sub = env.submissions.findById(sid);
    CHECK(sub->state == SubmissionState::Marked);
    auto mark = env.marks.findBySubmission(sid);
    REQUIRE(mark.has_value());
    auto events = env.auditRepo.listByMark(mark->id);
    REQUIRE(events.size() == 1);
    CHECK(events[0].type == AuditEventType::Initial);
    CHECK_FALSE(events[0].hasOldValue);
    CHECK(events[0].newValue == 80);
    CHECK(events[0].reason == "Initial mark entry");
    CHECK(events[0].actingUserId == ids.lecturerId);
}

TEST_CASE("The late penalty is applied at marking, not at submission") {
    TestEnv env;
    auto ids = env.seedWorld("2026-02-01T00:00:00");
    auto student = env.userById(ids.studentId);
    env.clock.set("2026-02-01T12:00:00");                      // 1 day late
    env.submissionSvc.submit(*student, ids.assignmentId, "late work");
    const int64_t sid = env.submissions.find(ids.assignmentId, ids.studentId)->id;

    auto lecturer = env.userById(ids.lecturerId);
    auto result = env.markingSvc.mark(*lecturer, sid, 80, "ok");
    CHECK(result.daysLate == 1);
    CHECK(result.awardedMark == 70);                           // 80 - 10% of 100
}

TEST_CASE("Re-marking requires a reason and writes a CHANGE audit event") {
    TestEnv env;
    auto ids = env.seedWorld();
    const int64_t sid = onTimeSubmission(env, ids);
    auto lecturer = env.userById(ids.lecturerId);
    env.markingSvc.mark(*lecturer, sid, 80, "first pass");

    CHECK_THROWS_AS(env.markingSvc.mark(*lecturer, sid, 90, "second pass", ""), ValidationError);

    env.markingSvc.mark(*lecturer, sid, 90, "second pass", "corrected an arithmetic slip");
    auto mark = env.marks.findBySubmission(sid);
    CHECK(mark->awardedMark == 90);
    auto events = env.auditRepo.listByMark(mark->id);
    REQUIRE(events.size() == 2);
    CHECK(events[1].type == AuditEventType::Change);
    CHECK(events[1].oldValue == 80);
    CHECK(events[1].newValue == 90);
}

TEST_CASE("Marking rejects out-of-range marks, empty feedback, and unassigned lecturers") {
    TestEnv env;
    auto ids = env.seedWorld();
    const int64_t sid = onTimeSubmission(env, ids);
    auto lecturer = env.userById(ids.lecturerId);

    CHECK_THROWS_AS(env.markingSvc.mark(*lecturer, sid, 101, "too high"), ValidationError);
    CHECK_THROWS_AS(env.markingSvc.mark(*lecturer, sid, -1, "negative"), ValidationError);
    CHECK_THROWS_AS(env.markingSvc.mark(*lecturer, sid, 50, "   "), ValidationError);

    const int64_t otherLecturerId =
        env.userAdmin.createUser(*env.admin, Role::Lecturer, "other", "password1", "Other");
    auto otherLecturer = env.userById(otherLecturerId);
    CHECK_THROWS_AS(env.markingSvc.mark(*otherLecturer, sid, 50, "x"), AuthorizationError);
}
