# Requirements Engineering — Part 3: Use Case Model

**System:** Moderated Learning Management System · **Author:** Mweli Pama (201404452)

All quantitative rules (D_e, days late d, raw mark R, penalty P, awarded mark A, cutoff) follow Stakeholder Analysis Section 3 and are used identically here.

## 1. Use case diagram

The diagram covers all four actors (Administrator, Lecturer, Head of Department, Student), each a specialisation of an abstract *User* (generalisation), with `<<include>>` and `<<extend>>` relationships.

**Figure 1. Use Case Diagram** — [assets/diagrams/use_case_diagram.png](../../assets/diagrams/use_case_diagram.png) · source: [use_case_diagram.puml](../../assets/diagrams/use_case_diagram.puml)

**Relationships (and why each is modelled this way):**
- **Generalisation:** Administrator, Lecturer, Head of Department and Student each generalise *User* (all authenticate via *Log in*).
- **`<<include>>` — *Mark Submission* includes *Write Feedback*:** feedback is always recorded when marking (FR-14).
- **`<<include>>` — *Mark Submission* includes *Record Mark Audit Event*:** marking always writes an audit event — **INITIAL** on the first mark (empty old value) or **CHANGE** on a re-mark (FR-17). Because it always occurs, `<<include>>` is correct.
- **`<<extend>>` — *Apply Late Penalty* extends *Mark Submission*:** lateness is *recorded* at submission, but the penalty is *calculated when the lecturer marks the submission* and only when the submission is Late. Conditional behaviour ⇒ `<<extend>>`.
- **`<<extend>>` — *Record Mark Audit Event* extends *Moderate Marks*:** moderation writes an audit event **only when the HoD changes a mark**. A moderation that makes no change writes no audit event; its outcome is the submission's transition to the Moderated state. Conditional ⇒ `<<extend>>`.
- **`<<extend>>` — *Resubmit before Deadline* extends *Submit Assignment*:** resubmission is an optional, conditional path of submitting (a prior submission exists and D_e has not passed).

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
| UC-08 | Mark Submission (incl. Write Feedback, Record Mark Audit Event; ext. Apply Late Penalty) | Lecturer |
| UC-09 | View Courses and Assignments | Student |
| UC-10 | Submit Assignment (ext. Resubmit before Deadline) | Student |
| UC-11 | View Released Marks and Feedback | Student |
| UC-12/13 | Moderate and Release Marks (ext. Record Mark Audit Event) | Head of Department |

Three use cases are specified in full below, including **Submit Assignment** as required.

---

## 3. Fully dressed use cases

### UC-10 — Submit Assignment

- **Primary actor:** Student
- **Stakeholders & interests:** Student (work recorded; can fix mistakes before the deadline; late work not rejected; an existing submission is protected from replacement after D_e); Lecturer (a single current submission to mark); HoD (late penalties enforceable).
- **Preconditions:** The student is authenticated (UC-01) and enrolled; the assignment exists; the current time is on or before the final acceptance cutoff (D_e + 72h).
- **Trigger:** The student chooses to submit work for an assignment.
- **Main success flow (on time):**
  1. The student selects an assignment from their list.
  2. The system shows the maximum mark M and the effective deadline D_e (extension if granted, else the assignment deadline — C5).
  3. The student provides their submission content/reference.
  4. The system records the submission with the current timestamp, linked to the student and assignment.
  5. As the timestamp ≤ D_e, the system sets the state to **Submitted** (FR-18) and confirms success.
- **Alternative flows:**
  - *2a. Extension exists:* the D_e shown is the student's individual extension (FR-09).
  - *3a. Resubmission before D_e:* if a submission already exists and the timestamp ≤ D_e, the new submission **replaces** the previous one (FR-12a); state remains **Submitted**; no penalty.
  - *4a. First submission after D_e, within the cutoff, no earlier submission exists:* the system accepts the work, records its lateness `d = ceil((timestamp − D_e)/24h)`, and sets the state to **Late** (FR-12c). **No penalty is calculated here** — lateness is only recorded; the penalty is applied at marking (UC-08).
- **Exception flows:**
  - *E1. Any existing submission (on-time or Late), attempt after D_e:* the system **refuses** the replacement and preserves the existing submission (FR-12b, C7). An on-time submission stands and is marked without penalty; an existing Late submission is likewise not replaced and keeps its recorded lateness `d`. A first Late submission is accepted only when no submission exists (step 4a).
  - *E2. Attempt after the final acceptance cutoff:* refused; if no submission exists, the assignment is recorded as not submitted and scored zero (FR-21).
  - *E3. Not enrolled / assignment not visible:* refused with an authorisation message (FR-02).
  - *E4. Invalid or empty content:* rejected and re-entered (FR-19).
  - *E5. Data-access error:* caught and reported; no partial submission (FR-19, NFR-06).
- **Postconditions:** At most one current submission exists for the student and assignment, in state **Submitted** (on time) or **Late** (with recorded `d`); no penalty has yet been computed; no mark is visible to the student (FR-16).
- **Business rules:** C2, C5, C6, C7; FR-12, FR-13 (penalty deferred to marking), FR-18, FR-21.

### UC-08 — Mark Submission

- **Primary actor:** Lecturer
- **Stakeholders & interests:** Lecturer (mark quickly, give feedback); Student (written feedback, not just a number; correct penalty); HoD/Principal (nothing visible to the student yet); school (every mark-affecting action audited).
- **Preconditions:** The lecturer is authenticated and assigned to the course; a submission exists for the assignment.
- **Trigger:** The lecturer chooses to mark a submission.
- **Main success flow:**
  1. The lecturer selects a submission to mark.
  2. The system shows the submission, the maximum mark M, and the submission's state (Submitted or Late with its recorded `d`).
  3. The lecturer enters a raw mark R (0..M) and written feedback.
  4. The system validates R's range and that feedback is non-empty (FR-14).
  5. **If the submission is Late, `<<extend>>` Apply Late Penalty:** the system computes `A = clamp(R − 0.10·M·d, 0, M)` rounded to the nearest whole mark (FR-13). If on time, `A = R`.
  6. The system stores R, A and feedback and sets the state to **Marked** (FR-18).
  7. **`<<include>>` Record Mark Audit Event:** on the first mark the system writes an **INITIAL** event with a **system-generated reason** ("Initial mark entry"), old value empty, new value A, acting lecturer and timestamp. On a re-mark (see 3a) it writes a **CHANGE** event carrying the **lecturer-entered reason**, old A → new A (FR-17).
  8. The mark remains invisible to the student until release (FR-16).
- **Alternative flows:**
  - *3a. Re-marking an already-marked submission (before release):* permitted; before saving, the system requires the lecturer to enter a **reason** for the change; step 7 then writes a **CHANGE** event with the previous A as the old value and that reason. (The first mark needs no reason — its INITIAL reason is system-generated.)
- **Exception flows:**
  - *E1. R out of range or feedback empty:* rejected and re-entered (FR-14, FR-19).
  - *E2. Lecturer not assigned to the course:* refused (FR-02).
  - *E3. Re-mark with an empty reason:* rejected; the revised mark is not saved until a reason is provided (FR-17).
  - *E4. Data-access error:* caught and reported; no partial write (NFR-06).
- **Postconditions:** The submission is **Marked** with raw mark R, awarded mark A and feedback; exactly one audit event (INITIAL with a system reason, or CHANGE with the lecturer's reason) exists for this action; the student still sees nothing (FR-16).
- **Business rules:** FR-13 (penalty at marking), FR-14 (range + feedback), FR-16 (no early visibility), FR-17 (INITIAL/CHANGE audit).

### UC-12/13 — Moderate and Release Marks

- **Primary actor:** Head of Department
- **Stakeholders & interests:** HoD (sees every mark before release; quality control); Principal (no mark visible until checked); Lecturer (marks respected unless adjustment justified); Student (eventually sees a fair, released mark and feedback).
- **Preconditions:** The HoD is authenticated; the relevant submissions are in state **Marked**; the courses belong to the HoD's department.
- **Trigger:** The HoD chooses to moderate the marks for an assignment or course in their department.
- **Main success flow:**
  1. The HoD selects a department course and assignment.
  2. The system lists the marked submissions with their awarded marks A and feedback.
  3. The HoD reviews each mark.
  4. **For a mark the HoD changes:** the HoD enters a new value and a reason; the system stores it, and `<<extend>>` **Record Mark Audit Event** writes a **MODERATION_CHANGE** event (old A → new A, acting HoD, timestamp, reason) (FR-15, FR-17); the submission state becomes **Moderated** (FR-18).
  5. **For a mark the HoD accepts unchanged:** the system sets the state to **Moderated** and writes **no** mark audit event — the moderation outcome is recorded by the state transition itself.
  6. When satisfied, the HoD **releases** the marks for the assignment.
  7. The system sets the released submissions' state to **Released** and makes the marks and feedback visible to the affected students (FR-16).
  8. The system confirms the release.
- **Alternative flows:**
  - *6a. Partial release:* the HoD releases only the submissions reviewed so far; the rest remain **Moderated** or **Marked**.
- **Exception flows:**
  - *E1. Course outside the HoD's department:* access refused (FR-02).
  - *E2. Attempt to release a submission not yet Moderated:* rejected — only Moderated submissions may be released (FR-18).
  - *E3. Adjusted mark out of range:* rejected and re-entered (FR-14).
  - *E4. Data-access error:* caught and reported; no partial state change (NFR-06).
- **Postconditions:** Reviewed submissions are **Moderated** (every decision recorded by state, changes additionally audited); released submissions are **Released** and visible to students; unreleased marks remain invisible.
- **Business rules:** C1 (marking separated from release), FR-15 (department-scoped moderation), FR-16 (controlled release), FR-17 (audit only on change; state records every decision), FR-18 (valid transitions).

---

## 4. Note on later gates

These use cases provide the basis for the Gate 2 behavioural models: the **Submission state machine** (Submitted/Late/Marked/Moderated/Released, FR-18), the **swimlane activity diagram** (submission → marking with penalty → moderation → release), and the **sequence diagrams** for *Submit Assignment* (UC-10, on-time, late and refused-replacement paths) and *Moderate and Release Marks* (UC-12/13, with and without adjustment).

## UC-20 Extend Class Deadline (CR1, v1.1)

- **Actor:** Lecturer. **Precondition:** the lecturer is assigned to the assignment's course.
- **Main flow:** the lecturer selects the assignment and enters a new class deadline; the system validates it and records one class extension for the assignment.
- **Postcondition:** every enrolled student's effective deadline is recalculated as the latest of base, class and individual deadlines; the change applies to both submission acceptance and the late penalty.
- **Exceptions:** E1 lecturer not assigned to the course → refused (authorisation); E2 invalid datetime → rejected.
- **Precedence:** an individual extension still applies; where both exist the student keeps whichever deadline is later, so no student is ever made worse off by a class extension.
