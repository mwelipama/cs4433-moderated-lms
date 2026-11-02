#pragma once
#include "domain/User.hpp"
#include "services/ServiceContainer.hpp"
#include <memory>

namespace lms {

// Drives the interactive session: a top-level login loop, then a menu whose options are
// determined by the authenticated user's role. The presentation layer performs no business
// logic and no SQL; it only gathers input, calls services, and renders results and errors.
class ConsoleApp {
public:
    explicit ConsoleApp(ServiceContainer& services) : svc_(services) {}
    void run();

private:
    std::unique_ptr<User> login();
    void dispatch(const User& user);

    void adminMenu(const User& user);
    void lecturerMenu(const User& user);
    void hodMenu(const User& user);
    void studentMenu(const User& user);

    // Administrator actions
    void adminCreateDepartment(const User& user);
    void adminListDepartments(const User& user);
    void adminCreateUser(const User& user);
    void adminListUsers(const User& user);
    void adminSetActive(const User& user);
    void adminCreateCourse(const User& user);
    void adminAssignLecturer(const User& user);
    void adminEnrolStudent(const User& user);
    void adminListCourses(const User& user);

    // Lecturer actions
    void lecturerListCourses(const User& user);
    void lecturerListAssignments(const User& user);
    void lecturerCreateAssignment(const User& user);
    void lecturerGrantExtension(const User& user);
    void lecturerExtendClass(const User& user);
    void lecturerMarkQueue(const User& user);
    void lecturerMark(const User& user);

    // Head of Department actions
    void hodListCourses(const User& user);
    void hodListAssignments(const User& user);
    void hodModerationQueue(const User& user);
    void hodAdjust(const User& user);
    void hodAccept(const User& user);
    void hodRelease(const User& user);

    // Student actions
    void studentListCourses(const User& user);
    void studentListAssignments(const User& user);
    void studentSubmit(const User& user);
    void studentViewSubmission(const User& user);
    void studentViewResult(const User& user);

    std::string nameOf(int64_t userId);

    ServiceContainer& svc_;
};

} // namespace lms
