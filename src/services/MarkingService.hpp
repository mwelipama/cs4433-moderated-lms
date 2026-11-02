#pragma once
#include "dao/Repositories.hpp"
#include "domain/PenaltyPolicy.hpp"
#include "services/AuditService.hpp"
#include "infra/Database.hpp"
#include <string>
#include <vector>

namespace lms {

// Lecturer marking: initial mark and re-mark. The late penalty is applied here (never at
// submission) through the injected PenaltyPolicy. Saving the mark + feedback, moving the
// submission to Marked, and writing the INITIAL/CHANGE audit event happen in one atomic
// transaction, so a mark can never exist without its audit event or vice versa.
class MarkingService {
public:
    struct MarkResult { int rawMark; int awardedMark; int daysLate; };

    MarkingService(ISubmissionRepository& submissions, IMarkRepository& marks,
                   IAssignmentRepository& assignments, ILecturerCourseRepository& lecturerCourses,
                   IExtensionRepository& extensions, AuditService& audit,
                   PenaltyPolicy& penaltyPolicy, Database& db)
        : submissions_(submissions), marks_(marks), assignments_(assignments),
          lecturerCourses_(lecturerCourses), extensions_(extensions), audit_(audit),
          penaltyPolicy_(penaltyPolicy), db_(db) {}

    // changeReason is required only when a mark already exists (a re-mark).
    MarkResult mark(const User& actor, int64_t submissionId, int rawMark,
                    const std::string& feedback, const std::string& changeReason = "");
    std::vector<Submission> markQueue(const User& actor, int64_t assignmentId);

private:
    Assignment requireMarkableByLecturer(const User& actor, const Submission& sub);
    ISubmissionRepository& submissions_;
    IMarkRepository& marks_;
    IAssignmentRepository& assignments_;
    ILecturerCourseRepository& lecturerCourses_;
    IExtensionRepository& extensions_;
    AuditService& audit_;
    PenaltyPolicy& penaltyPolicy_;
    Database& db_;
};

} // namespace lms
