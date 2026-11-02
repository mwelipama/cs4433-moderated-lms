#include "services/ModerationService.hpp"
#include "services/Validation.hpp"
#include "domain/Errors.hpp"

namespace lms {

Assignment ModerationService::requireHodForAssignment(const User& actor, int64_t assignmentId) {
    if (actor.role() != Role::Hod)
        throw AuthorizationError("only a Head of Department may perform this action");
    auto assignment = assignments_.findById(assignmentId);
    if (!assignment) throw NotFoundError("no assignment with id " + std::to_string(assignmentId));
    auto course = courses_.findById(assignment->courseId);
    if (!course) throw NotFoundError("the assignment's course no longer exists");
    if (course->deptId != actor.departmentId())
        throw AuthorizationError("this course is outside your department");
    return *assignment;
}

Assignment ModerationService::requireHodForSubmission(const User& actor, const Submission& sub) {
    return requireHodForAssignment(actor, sub.assignmentId);
}

void ModerationService::adjust(const User& actor, int64_t submissionId, int newAwardedMark,
                               const std::string& reason) {
    auto sub = submissions_.findById(submissionId);
    if (!sub) throw NotFoundError("no submission with id " + std::to_string(submissionId));
    const Assignment assignment = requireHodForSubmission(actor, *sub);
    if (sub->state != SubmissionState::Marked)
        throw ConflictError("only a marked submission can be moderated");
    auto mark = marks_.findBySubmission(submissionId);
    if (!mark) throw NotFoundError("this submission has no mark to moderate");
    requireRange(newAwardedMark, 0, assignment.maxMark, "moderated mark");
    requireNonBlank(reason, "moderation reason");

    const int64_t actingId = actor.id();
    db_.transaction([&] {
        const int oldValue = mark->awardedMark;
        marks_.update(mark->id, mark->rawMark, newAwardedMark, mark->feedback);
        audit_.recordModerationChange(mark->id, actingId, oldValue, newAwardedMark, reason);
        submissions_.setState(submissionId, SubmissionState::Moderated);
    });
}

void ModerationService::accept(const User& actor, int64_t submissionId) {
    auto sub = submissions_.findById(submissionId);
    if (!sub) throw NotFoundError("no submission with id " + std::to_string(submissionId));
    requireHodForSubmission(actor, *sub);
    if (sub->state != SubmissionState::Marked)
        throw ConflictError("only a marked submission can be moderated");
    db_.transaction([&] {
        submissions_.setState(submissionId, SubmissionState::Moderated);  // no audit event
    });
}

int ModerationService::release(const User& actor, int64_t assignmentId) {
    requireHodForAssignment(actor, assignmentId);
    int released = 0;
    db_.transaction([&] {
        for (const auto& s : submissions_.listByAssignment(assignmentId)) {
            if (s.state == SubmissionState::Moderated) {
                submissions_.setState(s.id, SubmissionState::Released);
                ++released;
            }
        }
    });
    return released;
}

std::vector<Submission> ModerationService::moderationQueue(const User& actor, int64_t assignmentId) {
    requireHodForAssignment(actor, assignmentId);
    std::vector<Submission> out;
    for (auto& s : submissions_.listByAssignment(assignmentId))
        if (s.state == SubmissionState::Marked) out.push_back(s);
    return out;
}

} // namespace lms
