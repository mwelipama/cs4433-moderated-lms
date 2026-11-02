-- Moderated Learning Management System - SQLite schema
-- Design artefact (DDL only). No database is created or populated at the design stage.
-- Naming: snake_case tables/columns; datetimes stored as ISO-8601 TEXT (SQLite convention).

PRAGMA foreign_keys = ON;

CREATE TABLE departments (
    dept_id  INTEGER PRIMARY KEY,
    name     TEXT NOT NULL UNIQUE
);

-- Single-table inheritance for the User hierarchy; 'role' is the discriminator.
CREATE TABLE users (
    user_id       INTEGER PRIMARY KEY,
    username      TEXT    NOT NULL UNIQUE,
    password_hash TEXT    NOT NULL,                 -- Argon2id PHC string (libsodium); embeds salt+params
    display_name  TEXT    NOT NULL,
    role          TEXT    NOT NULL CHECK (role IN ('ADMIN','LECTURER','HOD','STUDENT')),
    active        INTEGER NOT NULL DEFAULT 1 CHECK (active IN (0,1)),
    department_id INTEGER REFERENCES departments(dept_id)   -- used for HoD; NULL for others
);

CREATE TABLE courses (
    course_id INTEGER PRIMARY KEY,
    code      TEXT    NOT NULL UNIQUE,
    title     TEXT    NOT NULL,
    dept_id   INTEGER NOT NULL REFERENCES departments(dept_id)
);

-- M:N  course <-> lecturer
CREATE TABLE lecturer_courses (
    course_id   INTEGER NOT NULL REFERENCES courses(course_id),
    lecturer_id INTEGER NOT NULL REFERENCES users(user_id),
    PRIMARY KEY (course_id, lecturer_id)
);

-- M:N  course <-> student
CREATE TABLE enrolments (
    course_id  INTEGER NOT NULL REFERENCES courses(course_id),
    student_id INTEGER NOT NULL REFERENCES users(user_id),
    PRIMARY KEY (course_id, student_id)
);

CREATE TABLE assignments (
    assignment_id INTEGER PRIMARY KEY,
    course_id     INTEGER NOT NULL REFERENCES courses(course_id),
    title         TEXT    NOT NULL,
    deadline      TEXT    NOT NULL,                 -- ISO-8601 datetime
    max_mark      INTEGER NOT NULL CHECK (max_mark > 0)
);

-- Individual extension (at most one per student per assignment)
CREATE TABLE extensions (
    extension_id      INTEGER PRIMARY KEY,
    assignment_id     INTEGER NOT NULL REFERENCES assignments(assignment_id),
    student_id        INTEGER NOT NULL REFERENCES users(user_id),
    extended_deadline TEXT    NOT NULL,
    UNIQUE (assignment_id, student_id)
);

-- One current submission per student per assignment (resubmission updates it)
CREATE TABLE submissions (
    submission_id INTEGER PRIMARY KEY,
    assignment_id INTEGER NOT NULL REFERENCES assignments(assignment_id),
    student_id    INTEGER NOT NULL REFERENCES users(user_id),
    content       TEXT    NOT NULL,
    submitted_at  TEXT    NOT NULL,                 -- ISO-8601 datetime; lateness derived, not stored
    state         TEXT    NOT NULL CHECK (state IN ('SUBMITTED','LATE','MARKED','MODERATED','RELEASED')),
    UNIQUE (assignment_id, student_id)
);

CREATE TABLE marks (
    mark_id       INTEGER PRIMARY KEY,
    submission_id INTEGER NOT NULL UNIQUE REFERENCES submissions(submission_id),
    raw_mark      INTEGER NOT NULL CHECK (raw_mark >= 0),
    awarded_mark  INTEGER NOT NULL CHECK (awarded_mark >= 0),
    feedback      TEXT    NOT NULL
    -- Release is NOT stored here: visibility to students is derived from
    -- submissions.state = 'RELEASED' (the single authoritative release state).
);

-- Append-only audit trail of mark-affecting actions
CREATE TABLE audit_events (
    audit_id       INTEGER PRIMARY KEY,
    mark_id        INTEGER NOT NULL REFERENCES marks(mark_id),
    event_type     TEXT    NOT NULL CHECK (event_type IN ('INITIAL','CHANGE','MODERATION_CHANGE')),
    acting_user_id INTEGER NOT NULL REFERENCES users(user_id),
    event_time     TEXT    NOT NULL,
    old_value      INTEGER,                         -- NULL only for INITIAL
    new_value      INTEGER NOT NULL,
    reason         TEXT    NOT NULL,
    CHECK ( (event_type = 'INITIAL') = (old_value IS NULL) ),
    -- Reason semantics (FR-17): INITIAL is the fixed system text; CHANGE and
    -- MODERATION_CHANGE require a non-empty, non-whitespace user-entered reason.
    CHECK (
        (event_type = 'INITIAL' AND reason = 'Initial mark entry')
        OR (event_type IN ('CHANGE','MODERATION_CHANGE')
             AND length(trim(reason, char(32)||char(9)||char(10)||char(13)||char(11)||char(12))) > 0)
    )
);

-- Enforce audit immutability (append-only) at the database level
CREATE TRIGGER audit_no_update BEFORE UPDATE ON audit_events
BEGIN SELECT RAISE(ABORT, 'audit_events is append-only'); END;
CREATE TRIGGER audit_no_delete BEFORE DELETE ON audit_events
BEGIN SELECT RAISE(ABORT, 'audit_events is append-only'); END;

-- Indexes for the common access paths (supports NFR-01/NFR-02)
CREATE INDEX idx_submissions_assignment ON submissions(assignment_id);
CREATE INDEX idx_enrolments_student     ON enrolments(student_id);
CREATE INDEX idx_audit_mark             ON audit_events(mark_id);
