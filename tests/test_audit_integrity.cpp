#include "doctest.h"
#include "TestEnv.hpp"
#include "domain/Errors.hpp"
#include "infra/Database.hpp"

using namespace lms;

namespace {
int64_t markOnce(TestEnv& env, const TestEnv::Ids& ids) {
    auto student = env.userById(ids.studentId);
    env.clock.set("2026-01-20T00:00:00");
    env.submissionSvc.submit(*student, ids.assignmentId, "work");
    const int64_t sid = env.submissions.find(ids.assignmentId, ids.studentId)->id;
    auto lecturer = env.userById(ids.lecturerId);
    env.markingSvc.mark(*lecturer, sid, 80, "feedback");
    return env.marks.findBySubmission(sid)->id;
}
} // namespace

TEST_CASE("The audit trail is append-only: updates and deletes are rejected by the database") {
    TestEnv env;
    auto ids = env.seedWorld();
    markOnce(env, ids);
    CHECK_THROWS_AS(env.db.exec("UPDATE audit_events SET reason='tampered'"), DbError);
    CHECK_THROWS_AS(env.db.exec("DELETE FROM audit_events"), DbError);
}

TEST_CASE("A whitespace-only change reason is rejected at the database") {
    TestEnv env;
    auto ids = env.seedWorld();
    const int64_t markId = markOnce(env, ids);
    AuditEvent e;
    e.markId = markId; e.type = AuditEventType::Change; e.actingUserId = ids.lecturerId;
    e.eventTime = "2026-02-01T00:00:00"; e.hasOldValue = true; e.oldValue = 80; e.newValue = 85;
    e.reason = "\t  \n";                              // whitespace only
    CHECK_THROWS_AS(env.auditRepo.append(e), DbError);
}

TEST_CASE("A marking failure rolls back atomically, leaving no mark and no audit event") {
    TestEnv env;
    auto ids = env.seedWorld();
    auto student = env.userById(ids.studentId);
    env.clock.set("2026-01-20T00:00:00");
    env.submissionSvc.submit(*student, ids.assignmentId, "work");
    const int64_t sid = env.submissions.find(ids.assignmentId, ids.studentId)->id;
    auto lecturer = env.userById(ids.lecturerId);

    // First mark succeeds.
    env.markingSvc.mark(*lecturer, sid, 80, "feedback");
    // A re-mark with no reason fails inside the transaction after work would have begun;
    // the stored mark and its audit trail must be unchanged.
    CHECK_THROWS_AS(env.markingSvc.mark(*lecturer, sid, 95, "feedback", ""), ValidationError);
    auto mark = env.marks.findBySubmission(sid);
    CHECK(mark->awardedMark == 80);
    CHECK(env.auditRepo.listByMark(mark->id).size() == 1);
}
