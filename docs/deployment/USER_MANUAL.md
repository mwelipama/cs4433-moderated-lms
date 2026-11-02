# User Manual

Start the program (`./build/lms`), choose **1) Log in**, and enter your username and
password. The menu shown depends on your role. Choose **0** to log out, then **0** to exit.

## Administrator
1. Create department — name a department (needed before adding a Head of Department or a course).
2. List departments.
3. Create user — choose the role (Administrator, Lecturer, Head of Department, Student); a Head of Department must be attached to a department. Passwords must be at least eight characters.
4. List users.
5. Activate / deactivate user — an inactive account cannot log in.
6. Create course — code, title and the owning department.
7. Assign lecturer to course.
8. Enrol student in course.
9. List courses.

## Lecturer
1. My courses — the courses you are assigned to.
2. List assignments in a course.
3. Create assignment — title, deadline (YYYY-MM-DDTHH:MM:SS, UTC) and maximum mark.
4. Grant a student an extension — a later deadline for one student.
5. Extend the deadline for a whole class (CR1) — one new deadline for every student on the assignment.
6. View submissions to mark.
7. Mark a submission — enter the raw mark and feedback (feedback is required). Marking again asks for a reason; the late penalty is applied automatically.

## Head of Department
1. Courses in my department.
2. List assignments in a course.
3. View submissions to moderate (marked submissions).
4. Moderate: adjust a mark — enter the new mark and a reason.
5. Moderate: accept a mark unchanged.
6. Release marks for an assignment — makes the moderated marks visible to students.

## Student
1. My courses.
2. List assignments in a course — shows the effective deadline, including any extension.
3. Submit / resubmit work — you may resubmit freely until the deadline; late work is accepted within 72 hours only if you have not submitted before.
4. View my submission status.
5. View my released result — the mark and feedback, once the Head of Department has released them.
