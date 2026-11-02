#pragma once
#include "dao/Repositories.hpp"
#include "infra/Clock.hpp"
#include <optional>
#include <string>

namespace lms {

// Student submission workflow, including the exact resubmission and first-late rules.
class SubmissionService {
public:
    // Awarded marks become visible only through myReleasedResult, after release.
    struct SubmitResult { SubmissionState state; int daysLate; };
    struct ReleasedResult { Submission submission; Mark mark; };

    SubmissionService(ISubmissionRepository& submissions, IAssignmentRepository& assignments,
                      IEnrolmentRepository& enrolments, IExtensionRepository& extensions,
                      IClassExtensionRepository& classExtensions,
                      IMarkRepository& marks, Clock& clock)
        : submissions_(submissions), assignments_(assignments), enrolments_(enrolments),
          extensions_(extensions), classExtensions_(classExtensions), marks_(marks), clock_(clock) {}

    SubmitResult submit(const User& actor, int64_t assignmentId, const std::string& content);
    std::optional<Submission> myGet(const User& actor, int64_t assignmentId);
    std::optional<ReleasedResult> myReleasedResult(const User& actor, int64_t assignmentId);

    // Effective deadline for a student = their individual extension, else the base deadline.
    std::string effectiveDeadline(const Assignment& a, int64_t studentId);

private:
    static constexpr int64_t kLateWindowSeconds = 72 * 3600;  // 72-hour acceptance cutoff
    ISubmissionRepository& submissions_;
    IAssignmentRepository& assignments_;
    IEnrolmentRepository& enrolments_;
    IExtensionRepository& extensions_;
    IClassExtensionRepository& classExtensions_;
    IMarkRepository& marks_;
    Clock& clock_;
};

} // namespace lms
