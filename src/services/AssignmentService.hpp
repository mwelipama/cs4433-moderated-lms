#pragma once
#include "dao/Repositories.hpp"
#include <string>
#include <vector>

namespace lms {

// Lecturer management of assignments and individual extensions. A lecturer may act only
// on courses they are assigned to; that ownership check lives here, in the service layer.
class AssignmentService {
public:
    AssignmentService(IAssignmentRepository& assignments, ICourseRepository& courses,
                      ILecturerCourseRepository& lecturerCourses,
                      IEnrolmentRepository& enrolments, IExtensionRepository& extensions,
                      IClassExtensionRepository& classExtensions)
        : assignments_(assignments), courses_(courses), lecturerCourses_(lecturerCourses),
          enrolments_(enrolments), extensions_(extensions), classExtensions_(classExtensions) {}

    int64_t createAssignment(const User& actor, int64_t courseId, const std::string& title,
                             const std::string& deadlineIso, int maxMark);
    void grantExtension(const User& actor, int64_t assignmentId, int64_t studentId,
                        const std::string& extendedDeadlineIso);
    // CR1: extend the deadline for every student taking the assignment.
    void extendClass(const User& actor, int64_t assignmentId, const std::string& extendedDeadlineIso);
    std::vector<Assignment> listAssignments(const User& actor, int64_t courseId);
    // Authorisation-enforcing listings for the read-only console paths:
    std::vector<Assignment> listForStudent(const User& actor, int64_t courseId);
    std::vector<Assignment> listForHod(const User& actor, int64_t courseId);

private:
    void requireLecturerForCourse(const User& actor, int64_t courseId) const;
    IAssignmentRepository& assignments_;
    ICourseRepository& courses_;
    ILecturerCourseRepository& lecturerCourses_;
    IEnrolmentRepository& enrolments_;
    IExtensionRepository& extensions_;
    IClassExtensionRepository& classExtensions_;
};

} // namespace lms
