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
    void dispatch(User& user);

    void adminMenu(User& user);
    void lecturerMenu(User& user);
    void hodMenu(User& user);
    void studentMenu(User& user);

    // Administrator actions
    void adminCreateDepartment(User& user);
    void adminListDepartments(User& user);
    void adminCreateUser(User& user);
    void adminListUsers(User& user);
    void adminSetActive(User& user);
    void adminCreateCourse(User& user);
    void adminAssignLecturer(User& user);
    void adminEnrolStudent(User& user);
    void adminListCourses(User& user);

    // Lecturer actions
    void lecturerListCourses(User& user);
    void lecturerListAssignments(User& user);
    void lecturerCreateAssignment(User& user);
    void lecturerGrantExtension(User& user);
    void lecturerMarkQueue(User& user);
    void lecturerMark(User& user);

    // Head of Department actions
    void hodListCourses(User& user);
    void hodListAssignments(User& user);
    void hodModerationQueue(User& user);
    void hodAdjust(User& user);
    void hodAccept(User& user);
    void hodRelease(User& user);

    // Student actions
    void studentListCourses(User& user);
    void studentListAssignments(User& user);
    void studentSubmit(User& user);
    void studentViewSubmission(User& user);
    void studentViewResult(User& user);

    std::string nameOf(int64_t userId);

    ServiceContainer& svc_;
};

} // namespace lms
