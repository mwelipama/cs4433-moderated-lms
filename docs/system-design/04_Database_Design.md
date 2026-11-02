# System Design — Part 4: Database Design

**System:** Moderated Learning Management System · **Author:** Mweli Pama (201404452)

**Figure 7. Entity-Relationship Diagram (3NF)** — [assets/diagrams/erd.png](../../assets/diagrams/erd.png) · source: [erd.puml](../../assets/diagrams/erd.puml)
**SQL schema:** [database/schema.sql](../../database/schema.sql) (DDL; tables, keys, constraints, audit-immutability triggers, indexes).

## 1. Normalisation (at least 3NF)

- **1NF:** every column is atomic and there are no repeating groups; the two many-to-many relationships (course–lecturer, course–student) are represented by the association tables `lecturer_courses` and `enrolments` rather than repeating columns.
- **2NF:** the only composite keys are those association tables, which have **no** non-key attributes, so there can be no partial dependency; every other table has a single-column surrogate key.
- **3NF:** every non-key attribute depends only on its table's key, with no transitive dependencies. A department's `name` lives only in `departments` (referenced by `dept_id`), never copied into `courses`. **Derived values are not stored:** a submission's lateness and days-late are computed from `submitted_at` against the effective deadline, so they introduce no derived-attribute dependency. `awarded_mark` is stored because it is an authoritative value the HoD may set independently of `raw_mark`, not a value functionally determined by another non-key column.

## 2. Data dictionary

| Table | Column | Type | Key / constraint | Description |
|---|---|---|---|---|
| departments | dept_id | INTEGER | PK | Department identifier |
| | name | TEXT | NOT NULL, UNIQUE | Department name |
| users | user_id | INTEGER | PK | User identifier |
| | username | TEXT | NOT NULL, UNIQUE | Login name |
| | password_hash | TEXT | NOT NULL | Argon2id PHC string (salt+params embedded) |
| | display_name | TEXT | NOT NULL | Shown name |
| | role | TEXT | NOT NULL, CHECK in (ADMIN,LECTURER,HOD,STUDENT) | Role discriminator |
| | active | INTEGER | NOT NULL, CHECK in (0,1) | Account enabled |
| | department_id | INTEGER | FK→departments | HoD's department (NULL otherwise) |
| courses | course_id | INTEGER | PK | Course identifier |
| | code | TEXT | NOT NULL, UNIQUE | Course code |
| | title | TEXT | NOT NULL | Course title |
| | dept_id | INTEGER | NOT NULL, FK→departments | Owning department |
| lecturer_courses | course_id | INTEGER | PK, FK→courses | Course in the pairing |
| | lecturer_id | INTEGER | PK, FK→users | Assigned lecturer |
| enrolments | course_id | INTEGER | PK, FK→courses | Course in the enrolment |
| | student_id | INTEGER | PK, FK→users | Enrolled student |
| assignments | assignment_id | INTEGER | PK | Assignment identifier |
| | course_id | INTEGER | NOT NULL, FK→courses | Owning course |
| | title | TEXT | NOT NULL | Assignment title |
| | deadline | TEXT | NOT NULL | ISO-8601 base deadline |
| | max_mark | INTEGER | NOT NULL, CHECK > 0 | Maximum mark M |
| extensions | extension_id | INTEGER | PK | Extension identifier |
| | assignment_id | INTEGER | NOT NULL, FK→assignments | Assignment extended |
| | student_id | INTEGER | NOT NULL, FK→users | Student granted the extension |
| | extended_deadline | TEXT | NOT NULL | The student's D_e |
| | (assignment_id, student_id) | | UNIQUE | At most one extension per student/assignment |
| submissions | submission_id | INTEGER | PK | Submission identifier |
| | assignment_id | INTEGER | NOT NULL, FK→assignments | Assignment submitted to |
| | student_id | INTEGER | NOT NULL, FK→users | Submitting student |
| | content | TEXT | NOT NULL | Submission content/reference |
| | submitted_at | TEXT | NOT NULL | ISO-8601 submission timestamp |
| | state | TEXT | NOT NULL, CHECK in (SUBMITTED,LATE,MARKED,MODERATED,RELEASED) | Lifecycle state |
| | (assignment_id, student_id) | | UNIQUE | One current submission per student/assignment |
| marks | mark_id | INTEGER | PK | Mark identifier |
| | submission_id | INTEGER | NOT NULL, UNIQUE, FK→submissions | Marked submission |
| | raw_mark | INTEGER | NOT NULL, CHECK ≥ 0 | Lecturer's raw mark R |
| | awarded_mark | INTEGER | NOT NULL, CHECK ≥ 0 | Awarded mark A (after penalty / moderation) |
| | feedback | TEXT | NOT NULL | Written feedback |
| audit_events | audit_id | INTEGER | PK | Audit entry identifier |
| | mark_id | INTEGER | NOT NULL, FK→marks | Mark affected |
| | event_type | TEXT | NOT NULL, CHECK in (INITIAL,CHANGE,MODERATION_CHANGE) | Kind of action |
| | acting_user_id | INTEGER | NOT NULL, FK→users | Who acted |
| | event_time | TEXT | NOT NULL | When |
| | old_value | INTEGER | NULL only for INITIAL | Previous awarded value |
| | new_value | INTEGER | NOT NULL | New awarded value |
| | reason | TEXT | NOT NULL | System text (INITIAL) or user-entered reason |

Integrity highlights: `CHECK ((event_type='INITIAL') = (old_value IS NULL))` ties the audit model to the schema; triggers `audit_no_update` / `audit_no_delete` make `audit_events` append-only (NFR-07). A second `CHECK` enforces the reason rules — INITIAL must be exactly `Initial mark entry`, and CHANGE/MODERATION_CHANGE must have a non-empty, non-whitespace reason (FR-17). **Release is represented solely by `submissions.state = 'RELEASED'`; marks carry no separate released flag (single source of truth).** The schema has been validated against SQLite (see [database/validate_schema.py](../../database/validate_schema.py)): it applies cleanly, valid data is accepted, and every constraint rejects its matching invalid case (bad role/state, wrong/empty/whitespace audit reason, non-null INITIAL old value, append-only UPDATE/DELETE, duplicate submission, non-positive max mark).

## 3. Class-to-database mapping

| Design element | Database | Notes / justified difference |
|---|---|---|
| `User` + `Administrator`/`Lecturer`/`HeadOfDepartment`/`Student` | `users` (one table) | **Single-table inheritance** with `role` discriminator; subclasses add almost no state (only HoD's `department_id`, a nullable column). Chosen over one-table-per-subclass to keep authentication a single lookup with no joins. Polymorphism stays in code; the DB keeps a discriminator. |
| `Department`, `Course`, `Assignment`, `Extension`, `Submission`, `Mark`, `AuditEvent` | `departments`, `courses`, `assignments`, `extensions`, `submissions`, `marks`, `audit_events` | One table per entity. |
| Course–Lecturer, Course–Student associations | `lecturer_courses`, `enrolments` | M:N association tables (no owning class). |
| `Role`, `SubmissionState`, `AuditEventType` enums | `TEXT` + `CHECK` | Enumerations enforced as constrained text. |
| `Submission.daysLate()`, `effectiveDeadline()` | (not stored) | Computed from `submitted_at`, `deadline` and any extension — keeps 3NF. |
| `PenaltyPolicy`/`StandardPenaltyPolicy`, services, repositories, `UserFactory`, `PasswordHasher`, `Clock` | (not persisted) | Behavioural/infrastructure types; no tables. |
| Mark visibility / release | `submissions.state` | **No stored column on `marks`.** A mark is visible to its student exactly when its submission's `state` is `RELEASED`. Moderation (mark update + audit event + state change) and release each run inside a single transaction so they commit atomically. Release is partial — only `Moderated` submissions are released. |

## 4. Runtime and portability (no manual SQL, no database server)

SQLite is an **embedded, serverless** database: its engine is compiled into the application, and the whole database is a single local file. The system therefore needs **no database server** — in particular, no PostgreSQL, MySQL or any separate SQL tool — to build, run or assess.

`database/schema.sql` is a **design artefact for reading**. It is **not** run by hand: on first run the application creates its SQLite database file and applies this schema itself (built in Gate 3), so a marker runs only the compiled console application (via CMake and a C++17 compiler, the toolchain the brief mandates) and never executes any SQL or DDL. Every artefact in the repository — the reports (PDF), this documentation (Markdown) and all diagrams (PNG/SVG) — is viewable directly in the GitHub web interface without any additional software.

## CR1 (v1.1) — class_extensions

A `class_extensions` table (`assignment_id` PRIMARY KEY → `extended_deadline` TEXT) holds at most one class-wide extension per assignment. It is added by an idempotent `CREATE TABLE IF NOT EXISTS` migration so existing databases upgrade safely. The effective deadline for a student is computed as the latest of the base deadline, the class extension and the individual `extensions` row — a single function used by both submission acceptance and marking, so the two can never diverge.
