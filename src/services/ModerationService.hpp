#pragma once
#include "dao/Repositories.hpp"
#include "services/AuditService.hpp"
#include "infra/Database.hpp"
#include <string>
#include <vector>

namespace lms {

// Head of Department moderation and release. A HoD may act only on courses in their own
// department. Each moderation action is atomic; release moves only submissions already in
// the Moderated state to Released in a single transaction, so no partial release occurs.
// submissions.state is the sole authority for whether a mark is visible to a student.
class ModerationService {
public:
    ModerationService(ISubmissionRepository& submissions, IMarkRepository& marks,
                      IAssignmentRepository& assignments, ICourseRepository& courses,
                      AuditService& audit, Database& db)
        : submissions_(submissions), marks_(marks), assignments_(assignments),
          courses_(courses), audit_(audit), db_(db) {}

    void adjust(const User& actor, int64_t submissionId, int newAwardedMark,
                const std::string& reason);
    void accept(const User& actor, int64_t submissionId);   // moderate without changing the mark
    int release(const User& actor, int64_t assignmentId);   // returns number of submissions released
    std::vector<Submission> moderationQueue(const User& actor, int64_t assignmentId);

private:
    Assignment requireHodForSubmission(const User& actor, const Submission& sub);
    Assignment requireHodForAssignment(const User& actor, int64_t assignmentId);
    ISubmissionRepository& submissions_;
    IMarkRepository& marks_;
    IAssignmentRepository& assignments_;
    ICourseRepository& courses_;
    AuditService& audit_;
    Database& db_;
};

} // namespace lms
