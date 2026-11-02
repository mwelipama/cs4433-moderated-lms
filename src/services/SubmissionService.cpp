#include "services/SubmissionService.hpp"
#include "services/Validation.hpp"
#include "domain/Errors.hpp"
#include "infra/DateTime.hpp"

namespace lms {

std::string SubmissionService::effectiveDeadline(const Assignment& a, int64_t studentId) {
    auto ext = extensions_.find(a.id, studentId);
    return ext ? ext->extendedDeadline : a.deadline;
}

SubmissionService::SubmitResult SubmissionService::submit(const User& actor, int64_t assignmentId,
                                                          const std::string& content) {
    if (actor.role() != Role::Student)
        throw AuthorizationError("only a student may submit work");
    auto assignment = assignments_.findById(assignmentId);
    if (!assignment) throw NotFoundError("no assignment with id " + std::to_string(assignmentId));
    if (!enrolments_.isEnrolled(assignment->courseId, actor.id()))
        throw AuthorizationError("you are not enrolled in this assignment's course");
    requireNonBlank(content, "submission content");

    const int64_t effEpoch = DateTime::toEpoch(effectiveDeadline(*assignment, actor.id()));
    const int64_t cutoffEpoch = effEpoch + kLateWindowSeconds;
    const int64_t nowEpoch = clock_.nowEpoch();
    const std::string nowIso = clock_.nowIso();
    auto existing = submissions_.find(assignmentId, actor.id());

    if (nowEpoch <= effEpoch) {
        // On time: a student may resubmit any number of times; the latest replaces the prior.
        if (existing) {
            if (existing->state != SubmissionState::Submitted)
                throw ConflictError("your submission has already been marked and cannot be replaced");
            existing->content = content;
            existing->submittedAt = nowIso;
            existing->state = SubmissionState::Submitted;
            submissions_.update(*existing);
            return {SubmissionState::Submitted, 0};
        }
        Submission s;
        s.assignmentId = assignmentId; s.studentId = actor.id();
        s.content = content; s.submittedAt = nowIso; s.state = SubmissionState::Submitted;
        submissions_.insert(s);
        return {SubmissionState::Submitted, 0};
    }

    if (nowEpoch <= cutoffEpoch) {
        // After the deadline, within the 72-hour window: a first late submission is accepted
        // only when none exists. Any existing submission (on time or late) cannot be replaced.
        if (existing)
            throw ConflictError("the deadline has passed; your existing submission cannot be replaced");
        Submission s;
        s.assignmentId = assignmentId; s.studentId = actor.id();
        s.content = content; s.submittedAt = nowIso; s.state = SubmissionState::Late;
        submissions_.insert(s);
        return {SubmissionState::Late, DateTime::daysLate(nowEpoch, effEpoch)};
    }

    throw ConflictError("the submission window, including the 72-hour late period, has closed");
}

std::optional<Submission> SubmissionService::myGet(const User& actor, int64_t assignmentId) {
    if (actor.role() != Role::Student)
        throw AuthorizationError("only a student may view their own submission");
    return submissions_.find(assignmentId, actor.id());
}

std::optional<SubmissionService::ReleasedResult>
SubmissionService::myReleasedResult(const User& actor, int64_t assignmentId) {
    if (actor.role() != Role::Student)
        throw AuthorizationError("only a student may view their own result");
    auto sub = submissions_.find(assignmentId, actor.id());
    if (!sub || sub->state != SubmissionState::Released) return std::nullopt;  // not yet released
    auto mark = marks_.findBySubmission(sub->id);
    if (!mark) return std::nullopt;
    return ReleasedResult{*sub, *mark};
}

} // namespace lms
