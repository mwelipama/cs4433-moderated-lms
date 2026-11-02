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

## 3. Agreed definitions used across all artefacts

To remove ambiguity, the following definitions are fixed and used identically in the SRS and the use cases:

- **Effective deadline (D_e):** the student's individual extension for an assignment if one was granted, otherwise the assignment's deadline.
- **Submission timestamp:** the local system clock time at which a submission is accepted. A submission is **on time** if its timestamp is less than or equal to D_e (a submission made exactly at the deadline is on time).
- **Days late (d):** for a submission after D_e, `d = ceil((timestamp − D_e) / 24 hours)` — each started 24-hour period counts as one day. Thus `0 < elapsed ≤ 24h ⇒ d = 1`; `24h < elapsed ≤ 48h ⇒ d = 2`; `48h < elapsed ≤ 72h ⇒ d = 3`. On time ⇒ `d = 0`.
- **Final acceptance cutoff:** `D_e + 72 hours` (three days). Submissions are accepted only up to this cutoff.
- **Raw mark (R):** the lecturer's assessment of the work, in the range 0..maximum (M).
- **Penalty (P):** `P = 0.10 × M × d`.
- **Awarded mark (A):** `A = clamp(R − P, 0, M)`, rounded to the nearest whole mark with halves rounded up. If no submission is accepted by the cutoff, `A = 0`.

**Justification of the started-day interpretation:** the penalty enforces a deadline. A submission even minutes past D_e has missed the deadline, so counting each started 24-hour period is the deterministic, testable reading of "for each day it is late", and it avoids disputes about part-days. Capping acceptance at three days realises "after that it should receive zero" as a single clean rule rather than an accepted-but-zero edge state, while honouring "late work should not simply be rejected" within the three-day window.

## 4. Conflicts and ambiguities — analysis and resolution

| ID | Conflict / ambiguity | Parties | Resolution | Rationale |
|---|---|---|---|---|
| C1 | "Students get marks as soon as I have marked" vs "students must not see any mark until the HoD has checked it" | Lecturer vs Principal/HoD | **Separate marking from release.** The lecturer records marks immediately and can see their own entered marks at once; students see *nothing* until the HoD moderates and performs an explicit **release**. | Satisfies the lecturer (mark without delay) and the Principal/HoD (nothing visible before moderation). Release is the single controlled visibility gate (FR-16, UC-12/13). |
| C2 | "Late work should not simply be rejected" vs the HoD's penalty schedule | Student rep vs HoD | **Accept late work within the three-day window and apply the graduated penalty defined in Section 3.** The penalty is `0.10 × M` per **started day late** (`d`, Section 3), for `d = 1,2,3`; past the cutoff no submission is accepted and the mark is zero. | Both statements hold: work is never outright rejected within the window, and the HoD's schedule is enforced exactly (FR-13). The started-day rule is identical to C6, Section 3 and FR-13. |
| C3 | Resubmission and feedback expectations | Student rep | **Resubmission is allowed up to D_e (latest replaces previous). After D_e no existing submission may be replaced. Written feedback is mandatory with every mark.** | Encodes the student's requests while protecting an on-time submission from being spoiled by a later edit (FR-12, FR-14; see C7). |
| C4 | "Respond quickly", "work offline", "passwords protected" are vague | IT officer | **Make them measurable:** offline single-machine/LAN operation (NFR-05); login within 2 s on the defined reference environment with the 800-student benchmark dataset (NFR-01); salted Argon2id password hashing via libsodium (NFR-03). | "Fast" and "protected" are unverifiable; measurable NFRs with a defined test environment make them reproducible. |
| C5 | Which deadline governs lateness when an extension exists? | Lecturer / Student | **The effective deadline D_e (Section 3) governs lateness, the resubmission window and the acceptance cutoff.** | Removes the ambiguity between base deadline, extensions and the penalty clock (FR-09, FR-13). |
| C6 | Calendar days vs 24-hour periods; part-days | HoD | **Each started 24-hour period after D_e counts as one full day (`d = ceil(elapsed/24h)`), capped at three; exactly at D_e is on time; exactly 24h late is `d = 1`.** | Matches "for each day it is late" and makes the rule deterministic and testable. Identical to Section 3, C2 and FR-13. |
| C7 | Can a late submission replace an existing on-time one? | Student / Lecturer | **A first late submission is accepted only when no earlier submission exists (within the cutoff). After D_e no existing submission can be replaced — whether on-time or Late: an on-time submission stands and is marked without penalty, and an existing Late submission keeps its recorded lateness.** | Protects a student who met the deadline, while still accepting genuinely late work from students who had submitted nothing (FR-12, FR-13; aligns with "resubmit before the deadline"). |

## 5. Outcome

All five source statements are reconciled without dropping any party's core requirement, and every quantitative rule is defined once in Section 3 and reused unchanged in the SRS and use cases. The resolutions are carried into functional requirements (FR-09, FR-12, FR-13, FR-14, FR-16, FR-21) and non-functional requirements (NFR-01, NFR-03, NFR-05) and are exercised by the use cases in Part 3.
