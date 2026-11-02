# System Design — Part 5: Requirements-to-Design Traceability

**System:** Moderated Learning Management System · **Author:** Mweli Pama (201404452)

Each accepted requirement is traced to the design elements that realise it (classes, diagrams, tables). This extends the SRS traceability seed and is carried forward to source files and tests in later gates.

| Requirement | Use case | Design element(s) | Diagram | Table(s) |
|---|---|---|---|---|
| FR-01, FR-02 | UC-01 | AuthService; `User.role()` | Fig 2, 3 | users |
| FR-03, NFR-03 | UC-01 | PasswordHasher (Argon2id/libsodium) | Fig 2, 3 | users.password_hash |
| FR-04 | UC-02 | UserService; UserFactory; `User` hierarchy | Fig 3 | users |
| FR-05 | UC-03 | CourseService; Department, Course | Fig 3 | departments, courses |
| FR-06 | UC-04 | CourseService | Fig 3 | lecturer_courses |
| FR-07 | UC-05 | CourseService | Fig 3 | enrolments |
| FR-08 | UC-06 | AssignmentService; Assignment | Fig 3 | assignments |
| FR-09, C5 | UC-07 | AssignmentService; Extension; `Submission.effectiveDeadline()` | Fig 3 | extensions |
| FR-10 | UC-09 | SubmissionService (read) | Fig 3 | enrolments, assignments |
| FR-11, FR-12, FR-18, FR-21, C2, C6, C7 | UC-10 | SubmissionService; `Submission` (state, daysLate) | Fig 4, 5, 6a | submissions |
| FR-13 | UC-08 | MarkingService; PenaltyPolicy/StandardPenaltyPolicy (Strategy) | Fig 3, 5 | marks |
| FR-14 | UC-08 | MarkingService; Mark | Fig 5, 6a | marks |
| FR-15, FR-16 | UC-12/13 | ModerationService; Mark.released | Fig 4, 5, 6b | marks |
| FR-17 (INITIAL/CHANGE/MODERATION_CHANGE) | UC-08, UC-12/13 | AuditService; AuditEvent | Fig 4, 5, 6b | audit_events (+ triggers) |
| FR-18 | UC-10, UC-08, UC-12/13 | `SubmissionState`; state-machine transitions | Fig 4 | submissions.state |
| FR-19 | all | input validation in services; exception handling | — | — |
| FR-20 | UC-02, UC-12/13 | AuditService (read); UserService/ModerationService | Fig 2 | audit_events |
| NFR-04 | all data access | Repository/DAO layer (parameterised queries) | Fig 2, 3 | all |
| NFR-05 | — | single-writer SQLite; offline | Fig 2 | — |
| NFR-06 | all | exceptions in services; DAO error handling | — | — |
| NFR-07 | UC-08, UC-12/13 | append-only audit; DB triggers | Fig 2 | audit_events |
| NFR-09 | — | layered design; services testable via repository interfaces | Fig 2, 3 | — |
| NFR-01, NFR-02, NFR-10 | — | indexes; CMake build | Fig 7 | idx_* |
