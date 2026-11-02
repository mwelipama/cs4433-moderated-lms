#pragma once
#include "dao/Repositories.hpp"
#include <string>
#include <vector>

namespace lms {

// Administrator-only management of courses, lecturer assignments and student enrolments.
class CourseService {
public:
    CourseService(ICourseRepository& courses, IDepartmentRepository& departments,
                  IUserRepository& users, ILecturerCourseRepository& lecturerCourses,
                  IEnrolmentRepository& enrolments)
        : courses_(courses), departments_(departments), users_(users),
          lecturerCourses_(lecturerCourses), enrolments_(enrolments) {}

    int64_t createCourse(const User& actor, const std::string& code,
                         const std::string& title, int64_t departmentId);
    void assignLecturer(const User& actor, int64_t courseId, int64_t lecturerId);
    void enrolStudent(const User& actor, int64_t courseId, int64_t studentId);
    std::vector<Course> listCourses(const User& actor);

private:
    void requireAdmin(const User& actor) const;
    ICourseRepository& courses_;
    IDepartmentRepository& departments_;
    IUserRepository& users_;
    ILecturerCourseRepository& lecturerCourses_;
    IEnrolmentRepository& enrolments_;
};

} // namespace lms
