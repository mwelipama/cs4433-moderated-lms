#include "doctest.h"
#include "TestEnv.hpp"
#include "domain/Errors.hpp"

using namespace lms;

namespace {
// Submit on time and mark, returning the submission id now in state Marked.
int64_t markedSubmission(TestEnv& env, const TestEnv::Ids& ids, int64_t studentId, int raw) {
    auto student = env.userById(studentId);
    env.clock.set("2026-01-20T00:00:00");
    env.submissionSvc.submit(*student, ids.assignmentId, "work");
    const int64_t sid = env.submissions.find(ids.assignmentId, studentId)->id;
    auto lecturer = env.userById(ids.lecturerId);
    env.markingSvc.mark(*lecturer, sid, raw, "feedback");
    return sid;
}
} // namespace

TEST_CASE("Moderation adjustment writes a MODERATION_CHANGE audit and sets Moderated") {
    TestEnv env;
    auto ids = env.seedWorld();
    const int64_t sid = markedSubmission(env, ids, ids.studentId, 80);
    auto hod = env.userById(ids.hodId);

    env.moderationSvc.adjust(*hod, sid, 85, "aligned with the cohort");
    auto sub = env.submissions.findById(sid);
    CHECK(sub->state == SubmissionState::Moderated);
    auto mark = env.marks.findBySubmission(sid);
    CHECK(mark->awardedMark == 85);
    auto events = env.auditRepo.listByMark(mark->id);
    REQUIRE(events.size() == 2);
    CHECK(events[1].type == AuditEventType::ModerationChange);
    CHECK(events[1].oldValue == 80);
    CHECK(events[1].newValue == 85);
    CHECK(events[1].actingUserId == ids.hodId);
}

TEST_CASE("Accepting a mark unchanged moves to Moderated and writes no audit event") {
    TestEnv env;
    auto ids = env.seedWorld();
    const int64_t sid = markedSubmission(env, ids, ids.studentId, 80);
    auto hod = env.userById(ids.hodId);
    auto mark = env.marks.findBySubmission(sid);
    const size_t before = env.auditRepo.listByMark(mark->id).size();

    env.moderationSvc.accept(*hod, sid);
    CHECK(env.submissions.findById(sid)->state == SubmissionState::Moderated);
    CHECK(env.auditRepo.listByMark(mark->id).size() == before);  // unchanged
}

TEST_CASE("A HoD cannot moderate a course outside their department") {
    TestEnv env;
    auto ids = env.seedWorld();
    const int64_t sid = markedSubmission(env, ids, ids.studentId, 80);
    const int64_t otherDept = env.userAdmin.createDepartment(*env.admin, "Mathematics");
    const int64_t otherHodId =
        env.userAdmin.createUser(*env.admin, Role::Hod, "mathhod", "password1", "Maths Head", otherDept);
    auto otherHod = env.userById(otherHodId);
    CHECK_THROWS_AS(env.moderationSvc.adjust(*otherHod, sid, 90, "x"), AuthorizationError);
}

TEST_CASE("Only a marked submission can be moderated") {
    TestEnv env;
    auto ids = env.seedWorld();
    auto student = env.userById(ids.studentId);
    env.clock.set("2026-01-20T00:00:00");
    env.submissionSvc.submit(*student, ids.assignmentId, "work");  // state Submitted, not Marked
    const int64_t sid = env.submissions.find(ids.assignmentId, ids.studentId)->id;
    auto hod = env.userById(ids.hodId);
    CHECK_THROWS_AS(env.moderationSvc.adjust(*hod, sid, 50, "x"), ConflictError);
}

TEST_CASE("Release affects only Moderated submissions and makes results visible") {
    TestEnv env;
    auto ids = env.seedWorld();
    const int64_t student2 = env.addStudent("stud2", ids.course);

    const int64_t sid1 = markedSubmission(env, ids, ids.studentId, 80);
    const int64_t sid2 = markedSubmission(env, ids, student2, 60);
    auto hod = env.userById(ids.hodId);

    env.moderationSvc.accept(*hod, sid1);          // sid1 -> Moderated
    // sid2 is left Marked (not moderated)

    // Before release, the student cannot see their result.
    auto student = env.userById(ids.studentId);
    CHECK_FALSE(env.submissionSvc.myReleasedResult(*student, ids.assignmentId).has_value());

    const int released = env.moderationSvc.release(*hod, ids.assignmentId);
    CHECK(released == 1);                            // only the Moderated one
    CHECK(env.submissions.findById(sid1)->state == SubmissionState::Released);
    CHECK(env.submissions.findById(sid2)->state == SubmissionState::Marked);  // untouched

    // After release, the result becomes visible through the single release authority.
    auto result = env.submissionSvc.myReleasedResult(*student, ids.assignmentId);
    REQUIRE(result.has_value());
    CHECK(result->mark.awardedMark == 80);
}

TEST_CASE("A lecturer can no longer re-mark once a submission is moderated") {
    TestEnv env;
    auto ids = env.seedWorld();
    const int64_t sid = markedSubmission(env, ids, ids.studentId, 80);
    auto hod = env.userById(ids.hodId);
    env.moderationSvc.accept(*hod, sid);
    auto lecturer = env.userById(ids.lecturerId);
    CHECK_THROWS_AS(env.markingSvc.mark(*lecturer, sid, 90, "late change", "reason"), ConflictError);
}
