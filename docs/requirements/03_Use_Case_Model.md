# Requirements Engineering — Part 3: Use Case Model

**System:** Moderated Learning Management System · **Author:** Mweli Pama (201404452)

## 1. Use case diagram

The diagram covers all four actors (Administrator, Lecturer, Head of Department, Student), each a specialisation of an abstract *User* (generalisation), and uses `<<include>>` and `<<extend>>` relationships.

**Figure 1. Use Case Diagram** — [assets/diagrams/use_case_diagram.png](../../assets/diagrams/use_case_diagram.png) · source: [use_case_diagram.puml](../../assets/diagrams/use_case_diagram.puml)

Relationships shown:
- **Generalisation:** Administrator, Lecturer, Head of Department and Student each generalise *User* (all authenticate via *Log in*).
- **`<<include>>`:** *Mark Submission* includes *Write Feedback* and *Record Mark Change (audit)*; *Moderate Marks* includes *Record Mark Change (audit)*.
- **`<<extend>>`:** *Resubmit before Deadline* and *Apply Late Penalty* extend *Submit Assignment* (conditional behaviour).

## 2. Use case list

| ID | Use case | Primary actor |
|---|---|---|
| UC-01 | Log in | User (all roles) |
| UC-02 | Manage User Accounts | Administrator |
| UC-03 | Manage Departments and Courses | Administrator |
| UC-04 | Assign Lecturer to Course | Administrator |
| UC-05 | Enrol Student in Course | Administrator |
| UC-06 | Create Assignment | Lecturer |
| UC-07 | Grant Individual Extension | Lecturer |
| UC-08 | Mark Submission (incl. Write Feedback, Record Mark Change) | Lecturer |
| UC-09 | View Courses and Assignments | Student |
| UC-10 | Submit Assignment (ext. Resubmit, Apply Late Penalty) | Student |
| UC-11 | View Released Marks and Feedback | Student |
| UC-12/13 | Moderate and Release Marks | Head of Department |

Three use cases are specified in full below, including **Submit Assignment** as required.

---

## 3. Fully dressed use cases

### UC-10 — Submit Assignment

- **Primary actor:** Student
- **Stakeholders & interests:** Student (wants work recorded, able to fix mistakes before the deadline, late work not rejected); Lecturer (wants a single current submission to mark); HoD (wants late penalties enforced).
- **Preconditions:** The student is authenticated (UC-01) and enrolled in the course; the assignment exists and is open; the student can see it (UC-09).
- **Trigger:** The student chooses to submit work for an assignment.
- **Main success flow:**
  1. The student selects an assignment from their list.
  2. The system shows the assignment's maximum mark and effective deadline (extension if granted, else the assignment deadline — C5).
  3. The student provides their submission content/reference.
  4. The system records the submission, linked to the student and assignment, with a timestamp.
  5. The system sets the submission state to **Submitted** (FR-18) and confirms success.
- **Alternative flows:**
  - *2a. Extension exists:* the effective deadline shown is the student's individual extension (FR-09).
  - *3a. Resubmission before the deadline:* if a submission already exists and the effective deadline has not passed, the new submission **replaces** the previous one (FR-12); state remains **Submitted**.
  - *4a. Submission after the effective deadline (<< extend >> Apply Late Penalty):* the system accepts the work, records it, and sets the state to **Late**; lateness in started days (ceiling, C6) is stored for use at marking (FR-13). Work is never rejected (C2).
- **Exception flows:**
  - *E1. Not enrolled / assignment not visible:* the system refuses the submission with an authorisation message (FR-02).
  - *E2. Invalid or empty content:* the system rejects the input and prompts again (FR-19).
  - *E3. Data-access error:* the system catches the error, reports it, and leaves no partial submission (FR-19, NFR-06).
- **Postconditions:** A single current submission exists for the student and assignment, in state **Submitted** or **Late**, with a timestamp and (if late) the recorded lateness. No mark is visible to the student (FR-16).
- **Business rules:** C2 (accept late, penalise), C5 (effective deadline), C6 (started-day counting), FR-12 (latest replaces previous), FR-13 (penalty schedule).

### UC-08 — Mark Submission

- **Primary actor:** Lecturer
- **Stakeholders & interests:** Lecturer (mark quickly, give feedback); Student (wants written feedback, not just a number); HoD/Principal (nothing visible to the student yet); school (every change audited).
- **Preconditions:** The lecturer is authenticated and assigned to the course; a submission exists for the assignment.
- **Trigger:** The lecturer chooses to mark a submission.
- **Main success flow:**
  1. The lecturer selects a submission to mark.
  2. The system shows the submission, the maximum mark, and any recorded lateness.
  3. The lecturer enters a numeric raw mark (0..maximum) and written feedback.
  4. The system validates the mark range and that feedback is non-empty (FR-14).
  5. If the submission is **Late**, the system applies the late-penalty rule to compute the awarded mark (FR-13; `<<include>>` the penalty calculation).
  6. The system stores the awarded mark and feedback and sets the state to **Marked** (FR-18).
  7. The system writes an audit record (acting lecturer, timestamp, old value → new value, reason) — `<<include>>` Record Mark Change (FR-17).
  8. The mark remains invisible to the student until release (FR-16).
- **Alternative flows:**
  - *3a. Re-marking an already-marked submission:* permitted before release; step 7 records the change with the previous value as the old value.
- **Exception flows:**
  - *E1. Mark out of range or feedback empty:* rejected; the lecturer is prompted again (FR-14, FR-19).
  - *E2. Lecturer not assigned to the course:* refused (FR-02).
  - *E3. Data-access error:* caught and reported; no partial write (NFR-06).
- **Postconditions:** The submission is in state **Marked** with an awarded mark and feedback; exactly one audit record exists for the change; the student still sees nothing (FR-16).
- **Business rules:** FR-13 (penalty), FR-14 (range + feedback), FR-16 (no early visibility), FR-17 (audit).

### UC-12/13 — Moderate and Release Marks

- **Primary actor:** Head of Department
- **Stakeholders & interests:** HoD (sees every mark before release; quality control); Principal (no mark visible until checked); Lecturer (marks respected unless adjustment justified); Student (eventually sees a fair, released mark and feedback).
- **Preconditions:** The HoD is authenticated; the relevant submissions are in state **Marked**; the courses belong to the HoD's department.
- **Trigger:** The HoD chooses to moderate the marks for an assignment or course in their department.
- **Main success flow:**
  1. The HoD selects a department course and assignment.
  2. The system lists the marked submissions with their awarded marks and feedback.
  3. The HoD reviews each mark.
  4. For any mark requiring change, the HoD enters a new value and a reason; the system stores it and writes an audit record (`<<include>>` Record Mark Change, FR-15, FR-17) and sets the submission state to **Moderated** (FR-18).
  5. When satisfied, the HoD **releases** the marks for the assignment.
  6. The system sets the released submissions' state to **Released** and makes the marks and feedback visible to the affected students (FR-16).
  7. The system confirms the release.
- **Alternative flows:**
  - *4a. No change needed:* the HoD accepts the mark unchanged; the system sets the state to **Moderated** without an audit record (no value changed).
  - *5a. Partial release:* the HoD releases only the submissions reviewed so far; the rest remain **Moderated**/**Marked**.
- **Exception flows:**
  - *E1. Course outside the HoD's department:* access refused (FR-02).
  - *E2. Attempt to release an unmarked submission:* rejected — only **Moderated** (or reviewed **Marked**) submissions may be released (FR-18).
  - *E3. Adjusted mark out of range:* rejected and re-prompted (FR-14).
  - *E4. Data-access error:* caught and reported; no partial state change (NFR-06).
- **Postconditions:** Reviewed submissions are **Moderated**; released submissions are **Released** and visible to students; every mark change has an audit record; unreleased marks remain invisible.
- **Business rules:** C1 (marking separated from release), FR-15 (department-scoped moderation), FR-16 (controlled release), FR-17 (audit), FR-18 (valid lifecycle transitions).

---

## 4. Note on later gates

These three use cases provide the basis for the Gate 2 behavioural models: the **Submission state machine** (FR-18 states), the **swimlane activity diagram** (submission → marking → moderation → release), and the **sequence diagrams** for *Submit Assignment* (UC-10, including late and resubmission) and *Moderate and Release Marks* (UC-12/13).
