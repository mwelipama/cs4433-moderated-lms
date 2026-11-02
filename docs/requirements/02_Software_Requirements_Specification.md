# Software Requirements Specification (SRS)

**System:** Moderated Learning Management System · **Author:** Mweli Pama (201404452)
**Structure:** organised along ISO/IEC/IEEE 29148 (introduction, overall description, specific requirements, verification). Each requirement has a unique ID, a MoSCoW priority and a verifiable acceptance criterion. Quantitative rules follow the fixed definitions in Stakeholder Analysis Section 3 (effective deadline D_e, days late d, raw mark R, penalty P, awarded mark A) and are not restated here.

## 1. Introduction

**1.1 Purpose.** Specifies the requirements for a Moderated Learning Management System (LMS) for a secondary school that manages courses, assignments and assessment for four roles, with controlled mark moderation and release.

**1.2 Scope.** An offline C++17 console application with SQLite persistence, run on a single school computer or local network. In scope: role-based access, assignment lifecycle, submission/resubmission, extensions, late penalties, marking and feedback, HoD moderation and controlled release, and a mark-change audit trail. Out of scope: online payment, video conferencing, automated grading, plagiarism detection, and web/mobile interfaces.

**1.3 Definitions.** See Stakeholder Analysis Section 3 for D_e, submission timestamp, days late d, final acceptance cutoff, raw mark R, penalty P and awarded mark A. *Release* — the HoD action that makes a moderated mark visible to the student. *Mark audit event* — an immutable log entry for a mark-affecting action (see FR-17).

**1.4 Actors.** Administrator, Lecturer, Head of Department (HoD), Student — specialisations of an abstract *User* (Part 3, Figure 1).

## 2. Overall description

**2.1 Product perspective.** A standalone, layered application: presentation (console menus) → business logic → data-access (DAO) → SQLite. No external services.

**2.2 Operating / reference environment.** Target: a typical school desktop — x86-64 dual-core CPU ~2 GHz, 4 GB RAM, 5400-rpm HDD (no SSD assumed), Windows 10, application built in Release mode via the project's CMake configuration, single local SQLite database file. This is the **reference environment** against which NFR-01/NFR-02 are measured.

**2.3 Benchmark dataset.** For performance verification: 800 student accounts, ~40 courses, ~200 assignments and ~5,000 submissions, produced by a repeatable seed routine. NFR timings are measured on this dataset in the reference environment.

**2.4 Interfaces.** User interface: text/console, menu-driven. Data interface: a single SQLite database file accessed through the DAO layer. No network or hardware interfaces.

**2.5 Users and characteristics.** Non-expert staff and ~800 students; single role per user.

**2.6 Constraints.** C++17; SQLite; CMake build; offline; salted adaptive password hashing (NFR-03); parameterised queries (NFR-04); append-only audit trail.

**2.7 Assumptions and dependencies.** One school; one database file; each user holds exactly one role; the local system clock is the authority for deadlines and timestamps; SQLite is used in single-writer mode (NFR-05).

## 3. Functional requirements

| ID | Priority | Requirement | Acceptance criterion (verifiable) |
|---|---|---|---|
| FR-01 | Must | Every user must authenticate with a username and password before any other action. | Any role action attempted without a valid session is refused. |
| FR-02 | Must | The system restricts each user to the operations permitted for their single role. | Each role can invoke only its permitted operations; others are refused with an authorisation error. |
| FR-03 | Must | Passwords are stored only as salted one-way hashes; plaintext is never stored or displayed. | Database inspection shows only the encoded hash; no plaintext field exists. |
| FR-04 | Must | The Administrator can create, edit and deactivate user accounts and assign exactly one role to each. | Each operation changes the account record; a deactivated account cannot log in. |
| FR-05 | Must | The Administrator can create and maintain departments and courses. | Created/edited departments and courses persist and are retrievable. |
| FR-06 | Must | The Administrator can assign one or more lecturers to a course. | The assignment persists; the lecturer sees the course. |
| FR-07 | Must | The Administrator can enrol students in courses. | The enrolment persists; the student sees the course (FR-10). |
| FR-08 | Must | A Lecturer can create an assignment for a course with a title, a deadline and a maximum mark M. | The assignment persists with all three attributes; enrolled students can see it. |
| FR-09 | Must | A Lecturer can grant a specific student an extended deadline for a specific assignment, setting that student's D_e. | Lateness, the resubmission window and the cutoff for that student are measured against D_e. |
| FR-10 | Must | A Student can view their enrolled courses and the assignments for those courses, including each assignment's effective deadline D_e. | The listing shows only the student's courses and the correct D_e. |
| FR-11 | Must | A Student can submit work for an assignment in a course they are enrolled in, up to the final acceptance cutoff. | A submission record is created, linked to the student and assignment, with a timestamp; its on-time/late status follows Section 3. |
| FR-12 | Must | Resubmission rule: **(a)** up to D_e a student may resubmit any number of times and the latest submission replaces the previous one (state Submitted, no penalty); **(b)** after D_e an existing on-time submission **cannot** be replaced — it stands and is marked without penalty; **(c)** if no submission exists, a first submission after D_e but within the cutoff is accepted as **Late**; **(d)** after the cutoff no submission is accepted. | Before D_e, a new submission supersedes the prior. After D_e with an on-time submission present, replacement is refused and the on-time submission is preserved. With no prior submission, a within-cutoff submission is accepted and marked Late. After the cutoff, submission is refused. |
| FR-13 | Must | The awarded mark applies the penalty of Section 3: `A = clamp(R − 0.10·M·d, 0, M)`, rounded to the nearest whole mark (halves up); `d = ceil(elapsed/24h)` capped at 3; if no submission by the cutoff, `A = 0`. | For given R, M and timestamp, A equals the Section 3 computation; boundaries: on time ⇒ no penalty; exactly 24h late ⇒ d=1; exactly 72h late ⇒ d=3; beyond cutoff ⇒ A=0. |
| FR-14 | Must | A Lecturer can record a raw mark R (0..M) and written feedback for a submission. | R within range and persisted with non-empty feedback; invalid marks are rejected. |
| FR-15 | Must | A Head of Department can review and, where necessary, adjust the awarded marks for all assignments in their department's courses before release. | The HoD can read and change any in-department mark; changes are audited (FR-17). |
| FR-16 | Must | Marks and feedback are withheld from students until the HoD releases them; a student never sees an unreleased mark. | Before release, the student view shows no mark; after release, it shows the released mark and feedback. |
| FR-17 | Must | The system records an immutable, append-only **mark audit event** for every mark-affecting action: **INITIAL** (first mark entry — old value is empty), **CHANGE** (a lecturer re-marks before release — old→new), and **MODERATION_CHANGE** (the HoD adjusts a mark — old→new). Each event stores event type, acting user, timestamp, old value (empty for INITIAL), new value and reason. A moderation decision that makes **no** change writes no mark audit event; its outcome is recorded by the submission's transition to the Moderated state (FR-18). | Each INITIAL/CHANGE/MODERATION_CHANGE action creates exactly one audit record with all fields (old value empty only for INITIAL); no application path edits or deletes an audit record; an unchanged moderation produces a Moderated state and no audit record. |
| FR-18 | Must | Each submission is tracked through the states Submitted, Late, Marked, Moderated and Released, permitting only valid transitions. | Invalid transitions are rejected; the stored state matches the lifecycle. |
| FR-19 | Should | The system validates all inputs and handles data-access errors via exceptions, reporting them without terminating. | Invalid input is rejected with a message; induced database errors are caught; the program remains usable. |
| FR-20 | Should | The Administrator and the relevant HoD can view the full audit trail for a mark. | The audit history for a selected mark is displayed in full. |
| FR-21 | Must | Submissions are accepted only up to the final acceptance cutoff (D_e + 72h). After the cutoff, no submission is accepted, and an assignment with no accepted submission is recorded as not submitted and scored zero. | A submission attempted after the cutoff is refused; an assignment with no submission by the cutoff yields A = 0. |

(21 functional requirements; the minimum required is 12.)

## 4. Non-functional requirements

| ID | Priority | Requirement (measurable) | Acceptance criterion |
|---|---|---|---|
| NFR-01 | Must | On the reference environment (2.2) with the benchmark dataset (2.3), a login attempt completes within 2 seconds. | Timed login on the reference environment/dataset is < 2 s (median of 10 runs). |
| NFR-02 | Should | On the reference environment with the benchmark dataset, listing a student's assignments or a lecturer's submissions returns within 2 seconds. | Timed listing on the reference environment/dataset is < 2 s (median of 10 runs). |
| NFR-03 | Must | Passwords are hashed with Argon2id via **libsodium** `crypto_pwhash`; the single self-contained PHC-format encoded string (which embeds the algorithm id, the opslimit/memlimit parameters and a per-user random salt of ≥ 16 bytes) is stored in the account's `password_hash` field; no plaintext and no separate salt column. | Code/library review confirms libsodium Argon2id use and the stored encoded-string format; verification succeeds against the stored string. |
| NFR-04 | Must | Every database statement using external input is parameterised; none is built by string concatenation of user input. | Static analysis / code review finds no concatenated SQL. |
| NFR-05 | Must | The system performs all functions with no internet connection, on a single computer or a local network, using SQLite in single-writer mode (one writing user at a time; concurrent multi-user writes are out of scope). | All use cases pass with networking disabled; concurrent-write access is not offered. |
| NFR-06 | Must | The application does not crash on invalid input or database errors; such conditions are caught and reported and the program stays usable. | Negative and boundary test cases do not terminate the process abnormally. |
| NFR-07 | Must | 100% of mark-affecting actions are audited per FR-17 and audit records cannot be edited or deleted through the application. | No application path modifies or deletes an audit record; coverage of INITIAL/CHANGE/MODERATION_CHANGE paths is complete. |
| NFR-08 | Should | In a moderated usability test with 5 representative first-time users (no training, without consulting documentation), at least 4 of 5 complete their core task (submit / mark / moderate and release) within 5 minutes; time-on-task and errors are recorded. | The test report shows ≥ 4/5 success within 5 minutes. |
| NFR-09 | Should | Code is organised into presentation, business-logic and data-access layers; the business-logic layer has at least 70% line coverage by automated tests. | Layering is evident; the coverage tool reports ≥ 70% on the business-logic layer. |
| NFR-10 | Should | The system builds from source with CMake and a C++17 compiler using only the documented build commands. | A clean checkout builds successfully following the build guide. |

(10 non-functional requirements; the minimum required is 6, all measurable against the reference environment where applicable.)

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
| FR-11, FR-12, FR-18, FR-21 | UC-10 Submit Assignment |
| FR-13, FR-14, FR-17 | UC-08 Mark Submission |
| FR-15, FR-16, FR-17, FR-18 | UC-12/13 Moderate and Release Marks |
| FR-16 | UC-11 View Released Marks and Feedback |
| FR-19, FR-20 | Cross-cutting (all use cases; UC-02, UC-12 for FR-20) |

## 6. Verification approach

Each requirement is verified by its acceptance criterion: functional requirements by use-case-derived system/acceptance tests and unit tests of the business rules (late penalty, lifecycle, audit); non-functional requirements by the stated measurements on the reference environment and by static analysis and code review (NFR-03/04/09). The full FR → use case → design → source → test traceability matrix is completed as later gates add design and code.
