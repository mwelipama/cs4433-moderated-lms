#include "services/MarkingService.hpp"
#include "services/Validation.hpp"
#include "domain/Errors.hpp"
#include "infra/DateTime.hpp"

namespace lms {

Assignment MarkingService::requireMarkableByLecturer(const User& actor, const Submission& sub) {
    if (actor.role() != Role::Lecturer)
        throw AuthorizationError("only a lecturer may mark a submission");
    auto assignment = assignments_.findById(sub.assignmentId);
    if (!assignment) throw NotFoundError("the submission's assignment no longer exists");
    if (!lecturerCourses_.isAssigned(assignment->courseId, actor.id()))
        throw AuthorizationError("you are not assigned to this submission's course");
    return *assignment;
}

MarkingService::MarkResult MarkingService::mark(const User& actor, int64_t submissionId,
                                                int rawMark, const std::string& feedback,
                                                const std::string& changeReason) {
    auto sub = submissions_.findById(submissionId);
    if (!sub) throw NotFoundError("no submission with id " + std::to_string(submissionId));
    const Assignment assignment = requireMarkableByLecturer(actor, *sub);

    if (sub->state == SubmissionState::Moderated || sub->state == SubmissionState::Released)
        throw ConflictError("this submission has been moderated and can no longer be marked by a lecturer");

    requireRange(rawMark, 0, assignment.maxMark, "mark");
    requireNonBlank(feedback, "feedback");

    const std::string effIso = extensions_.find(assignment.id, sub->studentId)
                                   ? extensions_.find(assignment.id, sub->studentId)->extendedDeadline
                                   : assignment.deadline;
    const int daysLate = sub->daysLate(effIso);
    const int awarded = penaltyPolicy_.awardedMark(rawMark, assignment.maxMark, daysLate);

    const int64_t actingId = actor.id();
    db_.transaction([&] {
        auto existing = marks_.findBySubmission(submissionId);
        if (!existing) {
            const int64_t markId = marks_.insert(submissionId, rawMark, awarded, feedback);
            submissions_.setState(submissionId, SubmissionState::Marked);
            audit_.recordInitial(markId, actingId, awarded);
        } else {
            if (isBlank(changeReason))
                throw ValidationError("a reason is required to change an existing mark");
            const int oldValue = existing->awardedMark;
            marks_.update(existing->id, rawMark, awarded, feedback);
            audit_.recordChange(existing->id, actingId, oldValue, awarded, changeReason);
        }
    });

    return {rawMark, awarded, daysLate};
}

std::vector<Submission> MarkingService::markQueue(const User& actor, int64_t assignmentId) {
    auto assignment = assignments_.findById(assignmentId);
    if (!assignment) throw NotFoundError("no assignment with id " + std::to_string(assignmentId));
    if (actor.role() != Role::Lecturer ||
        !lecturerCourses_.isAssigned(assignment->courseId, actor.id()))
        throw AuthorizationError("you are not assigned to this assignment's course");
    std::vector<Submission> out;
    for (auto& s : submissions_.listByAssignment(assignmentId)) {
        if (s.state == SubmissionState::Submitted || s.state == SubmissionState::Late ||
            s.state == SubmissionState::Marked)
            out.push_back(s);
    }
    return out;
}

} // namespace lms
