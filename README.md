# CS4433 — Moderated Learning Management System

Software Engineering individual project — a Moderated Learning Management System
developed in **C++17** on an embedded **SQLite** database.

- **Course:** CS4433 Software Engineering — National University of Lesotho
- **Author:** Mweli Pama (201404452)

## Scope

Four roles — Administrator, Lecturer, Head of Department, Student — with
role-based access control, assignment submission and resubmission, individual
extensions, late-penalty rules, Head-of-Department moderation and controlled
mark release, over persistent SQLite storage with an append-only mark-change
audit trail.

Out of scope: online payments, video conferencing, automated grading,
plagiarism detection, and web/mobile interfaces.

## Building

The only tools required are **CMake 3.16+** and a **C++17 compiler**
(GCC, Clang or MSVC). SQLite and the Argon2id password hasher are vendored under
`third_party/` and compiled from source, so there is nothing to install and no
network access is needed to build.

```
cmake -S . -B build
cmake --build build
```

This produces two executables in `build/`:

- `lms` — the console application
- `lms_tests` — the automated test suite

## Running

```
./build/lms            # uses ./lms.db, created on first run
./build/lms mydata.db  # or name the database file explicitly
```

On first run against an empty database the application applies its schema and
prompts you to create the initial administrator account. The database file
persists between runs. Log in, and the menu adapts to your role.

A typical end-to-end flow: the Administrator creates departments, users, courses,
lecturer assignments and enrolments; a Lecturer creates an assignment and marks
submissions; a Student submits work; the Head of Department moderates and
releases the marks; the Student then sees the released result.

## Testing

```
cd build
ctest --output-on-failure      # or run ./lms_tests directly for full output
```

The suite covers the late-penalty boundaries, the submission and resubmission
rules (on-time replacement, first-late acceptance, refusal of replacement after
the deadline, the 72-hour cutoff), marking and re-marking with audit events,
moderation and partial release, the append-only audit guarantees, and database
persistence across a reopen.

## Repository layout

```
src/
  infra/      database connection, prepared statements, datetime, password hashing, schema init
  domain/     User hierarchy, entities, enumerations, penalty policy (Strategy), user factory
  dao/        repository interfaces and their SQLite implementations
  services/   authentication, administration, submission, marking, moderation workflows
  ui/         console presentation (role-based menus, input helpers)
  app/        entry point
tests/        automated unit and integration tests
database/     schema.sql (the single source of truth for the schema) and its validator
third_party/  vendored SQLite, Argon2id and the test framework (with licences)
docs/         requirements, system design and project-management documents
assets/       diagrams (PlantUML sources rendered to PNG/SVG)
```

## Design notes

- **Layered architecture.** Presentation depends on services, services on
  repository interfaces and domain types; SQL is confined to the DAO layer and
  always parameterised.
- **Patterns.** Strategy for the late-penalty rule, Repository/DAO for
  persistence, Factory for constructing the correct `User` subclass.
- **Atomicity.** Marking, moderation and release each run in a single database
  transaction, so a mark and its audit event are always written together, and a
  release never partially commits.
- **Release authority.** A submission's `state` column is the sole authority for
  whether a mark is visible to a student; release is not stored on the mark.
- **Security.** Passwords are stored as Argon2id PHC strings (salt and
  parameters embedded); console password entry is not echoed.

## Phase 1 planning documents

The original project-planning deliverables remain in the repository root:
the Phase 1 management plan (`.docx`/`.pdf`), the schedule (`.xlsx`) and its
appendices.
