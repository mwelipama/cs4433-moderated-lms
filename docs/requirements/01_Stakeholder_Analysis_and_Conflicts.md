# Requirements Engineering — Part 1: Stakeholder Analysis and Conflict Resolution

**Project:** CS4433 Moderated Learning Management System · **Author:** Mweli Pama (201404452)

## 1. Stakeholder register

| Stakeholder | Type | Primary interest | Influence | Key concerns |
|---|---|---|---|---|
| Principal | Business owner / sponsor | A simple system; marks controlled before students see them | High | Simplicity for non-IT staff; no mark released without moderation |
| Head of Department (HoD) | Operational user | Quality control of marks across the department | High | Sees every mark before release; fair, enforced late rules |
| Lecturer | Operational user | Efficient marking; flexibility for genuine cases | High | Mark quickly; grant extensions; give written feedback |
| Student | End user | Fair treatment; chance to correct work; clear feedback | Medium | Resubmit before deadline; feedback not just a number; late work not rejected outright |
| IT officer | Technical / operations | A system that runs on the school's real hardware | Medium | Offline operation; password protection; adequate performance at 800 students |
| Administrator | Operational user (system) | Correct user, course and enrolment data | Medium | Reliable account, department, course and enrolment management |
| School / data subjects | Legal / affected party | Protection of learners' personal and assessment data | Medium (legal) | Confidentiality, auditability, lawful processing |

The four **actors** that interact with the software directly are Administrator, Lecturer, Head of Department and Student (all specialisations of an abstract *User*). The Principal, IT officer and school are stakeholders whose concerns shape requirements but who are not system actors.

## 2. Source statements (from the case study)

- **Principal:** "Teachers are not IT experts, so the system must be simple. Students must not see any mark until it has been checked by the Head of Department."
- **Lecturer:** "I want students to get their marks as soon as I have marked the work. Sometimes a student has a good reason for needing more time, and I should be able to give them an extension."
- **Head of Department:** "Late work should lose 10% of the available mark for each day it is late, for up to three days. After that it should receive zero. I need to see every mark in my department before it is released."
- **Student representative:** "Students should be able to fix a mistake and resubmit before the deadline, and we want written feedback, not just a number. Late work should not simply be rejected."
- **IT officer:** "Our computers are old and the internet is unreliable, so the system must work offline. Passwords must be protected, and the system should respond quickly even with all 800 students on it."

## 3. Conflicts and ambiguities — analysis and resolution

| ID | Conflict / ambiguity | Parties | Resolution | Rationale |
|---|---|---|---|---|
| C1 | "Students get marks as soon as I have marked" vs "students must not see any mark until the HoD has checked it" | Lecturer vs Principal/HoD | **Separate marking from release.** The lecturer records marks immediately and can see their own entered marks at once; students see *nothing* until the HoD moderates and performs an explicit **release**. | Satisfies the lecturer (mark without delay) and the Principal/HoD (nothing visible to students before moderation). Release becomes the single controlled visibility gate (FR-16, UC-12/13). |
| C2 | "Late work should not simply be rejected" vs "late work loses 10%/day up to 3 days, then zero" | Student rep vs HoD | **Accept late work, apply a graduated penalty.** Late submissions are accepted and marked; the penalty is 10% of the maximum mark per full day late up to three days; beyond three days the mark is zero (work still accepted and recorded, scored 0). | Both statements are satisfiable at once: the work is never outright rejected, and the HoD's penalty schedule is enforced exactly (FR-13). |
| C3 | Resubmission and feedback expectations are additive but unstated elsewhere | Student rep | **Allow resubmission up to the effective deadline; require written feedback with every mark.** The latest submission before the deadline replaces earlier ones; feedback is a mandatory text field alongside the numeric mark. | No conflict with other parties; encodes the student's reasonable requests as firm requirements (FR-12, FR-14). |
| C4 | "Respond quickly", "work offline", "passwords protected" are vague | IT officer | **Make them measurable.** Offline single-machine/LAN operation (NFR-05); login within 2 s with 800 students stored (NFR-01); salted adaptive password hashes (NFR-03). | "Fast" and "protected" are unverifiable; measurable NFRs make them testable. |
| C5 | Ambiguity: what is the "effective deadline" when an extension exists? | Lecturer / Student | **Effective deadline = the individual extension if one was granted for that student, otherwise the assignment deadline.** Lateness and the resubmission window are both measured against the effective deadline. | Removes the ambiguity between the base deadline, extensions and the late-penalty clock (FR-09, FR-13). |
| C6 | Ambiguity: does "10% per day" use calendar days or 24-hour periods, and is a part-day a full day? | HoD | **Each started day late counts as one full day** (ceiling of the lateness in 24-hour periods), capped at three. | Matches the common school interpretation ("for each day it is late") and makes the rule deterministic and testable. |

## 4. Outcome

All five source statements are reconciled without dropping any party's core requirement. The resolutions above are carried forward as functional requirements (FR-09, FR-12, FR-13, FR-14, FR-16) and non-functional requirements (NFR-01, NFR-03, NFR-05) in the SRS, and are exercised by the use cases in Part 3.
