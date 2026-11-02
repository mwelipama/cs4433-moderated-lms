# Software Requirements Specification (SRS)

**System:** Moderated Learning Management System · **Author:** Mweli Pama (201404452)
**Standard:** structured per ISO/IEC/IEEE 29148. Each requirement has a unique ID, a MoSCoW priority and a verifiable acceptance criterion.

## 1. Introduction

**1.1 Purpose.** This SRS specifies the requirements for a Moderated Learning Management System (LMS) for a secondary school that manages courses, assignments and assessment for four user roles, with controlled mark moderation and release.

**1.2 Scope.** The system is an offline C++17 console application with SQLite persistence, run on a single school computer or local network. In scope: role-based access, assignment lifecycle, submission/resubmission, extensions, late penalties, marking and feedback, HoD moderation and controlled release, and a mark-change audit trail. Out of scope: online payment, video conferencing, automated grading, plagiarism detection, and web/mobile interfaces.

**1.3 Definitions.** *Effective deadline* — the individual extension for a student if granted, else the assignment deadline. *Release* — the HoD action that makes a moderated mark visible to the student. *Audit record* — an immutable log entry for a mark change.

**1.4 Actors.** Administrator, Lecturer, Head of Department (HoD), Student — all specialisations of an abstract *User* (see Part 3, Figure 1).

## 2. Overall description

**2.1 Product perspective.** A standalone, layered application (presentation → business logic → data-access → SQLite). No external services.

**2.2 Users and characteristics.** Non-expert staff and ~800 students; menu-driven interaction; single role per user.

**2.3 Constraints.** C++17; SQLite; CMake build; offline; salted password hashing; parameterised queries; audit trail.

**2.4 Assumptions.** One school; one database file; each user holds exactly one role; system clock is the authority for deadlines.

## 3. Functional requirements

| ID | Priority | Requirement | Acceptance criterion (verifiable) |
|---|---|---|---|
| FR-01 | Must | Every user must authenticate with a username and password before any other action. | Any role action attempted without a valid session is refused. |
| FR-02 | Must | The system restricts each user to the operations permitted for their single role. | Each role can invoke only its permitted operations; all others are refused with an authorisation error. |
| FR-03 | Must | Passwords are stored only as salted one-way hashes; plaintext is never stored or displayed. | Database inspection shows only hash + salt; no plaintext field exists. |
| FR-04 | Must | The Administrator can create, edit and deactivate user accounts and assign exactly one role to each. | Each operation changes the account record accordingly; a deactivated account cannot log in. |
| FR-05 | Must | The Administrator can create and maintain departments and courses. | Created/edited departments and courses persist and are retrievable. |
| FR-06 | Must | The Administrator can assign one or more lecturers to a course. | The assignment persists; the lecturer sees the course. |
| FR-07 | Must | The Administrator can enrol students in courses. | The enrolment persists; the student sees the course (FR-10). |
| FR-08 | Must | A Lecturer can create an assignment for a course with a title, a deadline and a maximum mark. | The assignment persists with all three attributes; students enrolled in the course can see it. |
| FR-09 | Must | A Lecturer can grant a specific student an extended deadline for a specific assignment. | The student's effective deadline becomes the extension; lateness is measured against it. |
| FR-10 | Must | A Student can view their enrolled courses and the assignments for those courses, including each assignment's effective deadline. | The listing shows only the student's courses and the correct effective deadlines. |
| FR-11 | Must | A Student can submit work for an assignment in a course they are enrolled in. | A submission record is created and linked to the student and assignment. |
| FR-12 | Must | A Student can resubmit any number of times up to the effective deadline; the latest submission replaces the previous one. | Before the effective deadline, a new submission supersedes the prior; after it, resubmission is not accepted as on-time. |
| FR-13 | Must | The system reduces the awarded mark by 10% of the maximum mark for each started day a submission is late, up to three days; more than three days late scores zero. | For raw mark m, maximum M, lateness d days: awarded = max(0, m − ceil(d)×0.10×M) for 0<d≤3; awarded = 0 for d>3; awarded = m for d=0. |
| FR-14 | Must | A Lecturer can record a numeric mark (0..maximum) and written feedback for a submission. | The mark is within range and persisted with non-empty feedback; invalid marks are rejected. |
| FR-15 | Must | A Head of Department can review and, where necessary, adjust the marks for all assignments in their department's courses before release. | The HoD can read and change any in-department mark; changes are audited (FR-17). |
| FR-16 | Must | Marks and feedback are withheld from students until the HoD releases them; a student never sees an unreleased mark. | Before release, the student view shows no mark; after release, it shows the released mark and feedback. |
| FR-17 | Must | Every change to a mark is recorded with acting user, timestamp, old value, new value and reason; audit entries are append-only. | Each mark change creates exactly one immutable audit record containing all five fields. |
| FR-18 | Must | Each submission is tracked through the states Submitted, Late, Marked, Moderated and Released, permitting only valid transitions. | Attempted invalid transitions are rejected; the stored state matches the lifecycle. |
| FR-19 | Should | The system validates all inputs and handles data-access errors via exceptions, reporting them without terminating. | Invalid input is rejected with a message; induced database errors are caught; the program remains usable. |
| FR-20 | Should | The Administrator and the relevant HoD can view the audit trail for a mark. | The audit history for a selected mark is displayed in full. |

(20 functional requirements; the minimum required is 12.)

## 4. Non-functional requirements

| ID | Priority | Requirement (measurable) | Acceptance criterion |
|---|---|---|---|
| NFR-01 | Must | With 800 student accounts stored, a login attempt completes within 2 seconds on the target computer. | Timed login over an 800-account dataset is < 2 s. |
| NFR-02 | Should | With 800 students and their submissions stored, listing a student's assignments or a lecturer's submissions returns within 2 seconds. | Timed listing operation is < 2 s on the reference dataset. |
| NFR-03 | Must | Passwords are hashed with a recognised adaptive algorithm (e.g. bcrypt or Argon2) using a per-user random salt of at least 16 bytes. | Code/library review confirms the algorithm and salt length. |
| NFR-04 | Must | Every database statement using external input is parameterised; none is built by string concatenation of user input. | Static analysis / code review finds no concatenated SQL. |
| NFR-05 | Must | The system performs all functions with no internet connection, on a single computer or local network. | All use cases pass with networking disabled. |
| NFR-06 | Must | The application does not crash on invalid input or database errors; such conditions are caught and reported and the program stays usable. | Negative and boundary test cases do not terminate the process abnormally. |
| NFR-07 | Must | 100% of mark changes are recorded (FR-17) and audit records cannot be edited or deleted through the application. | No application path modifies or deletes an audit record; coverage of mark-change paths is complete. |
| NFR-08 | Should | A user can complete their core task (submit / mark / moderate and release) through the menu-driven interface without referring to documentation. | A first-time user completes the task in a usability walkthrough against the use cases. |
| NFR-09 | Should | Code is organised into presentation, business-logic and data-access layers; the business-logic layer has at least 70% line coverage by automated tests. | Layering is evident; the coverage tool reports >= 70% on the business-logic layer. |
| NFR-10 | Should | The system builds from source with CMake and a C++17 compiler using only the documented build commands. | A clean checkout builds successfully following the build guide. |

(10 non-functional requirements; the minimum required is 6, all measurable.)

## 5. Requirement-to-use-case traceability (seed)

| Requirement | Realised by use case(s) |
|---|---|
| FR-01, FR-02, FR-03 | UC-01 Log in |
| FR-04 | UC-02 Manage User Accounts |
| FR-05 | UC-03 Manage Departments and Courses |
| FR-06 | UC-04 Assign Lecturer to Course |
| FR-07 | UC-05 Enrol Student in Course |
| FR-08 | UC-06 Create Assignment |
| FR-09 | UC-07 Grant Individual Extension |
| FR-10 | UC-09 View Courses and Assignments |
| FR-11, FR-12, FR-13, FR-18 | UC-10 Submit Assignment |
| FR-14, FR-17 | UC-08 Mark Submission |
| FR-15, FR-16, FR-17, FR-18 | UC-12/13 Moderate and Release Marks |
| FR-16 | UC-11 View Released Marks and Feedback |
| FR-19, FR-20 | Cross-cutting (all use cases / UC-02, UC-12) |

This seed is extended into the full FR → use case → design → source → test traceability matrix as later gates add design and code.
