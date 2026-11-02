# System Design — Part 1: Architecture

**System:** Moderated Learning Management System · **Author:** Mweli Pama (201404452)

## 1. Architectural style: layered (chosen)

The system uses a **four-layer architecture**: Presentation (console) → Application Services (business logic) → Domain Model → Data Access (Repository/DAO) over SQLite, with a small Infrastructure layer (password hashing, clock, database connection). Dependencies point downward only; each layer depends on the interface of the layer below.

**Figure 2. Component Diagram** — [assets/diagrams/component_diagram.png](../../assets/diagrams/component_diagram.png) · source: [component_diagram.puml](../../assets/diagrams/component_diagram.puml)

**Why layered.** The assessment rules (role-based access, late penalties, moderation, controlled release, audit) are **business logic** that must be unit-testable independently of the console and of SQLite; a layered design isolates that logic in the service layer. Routing all persistence through a Data Access layer also gives a single place to enforce **parameterised queries** (NFR-04) and keeps SQL out of the business rules. It directly supports the required layering for the 70% business-logic coverage target (NFR-09).

**Alternative considered and rejected — Model-View-Controller (MVC).** MVC suits interfaces with rich, event-driven views and multiple synchronised presentations of the same model. This system has a single, sequential **console** interface with no concurrent views, so the controller/view separation would add indirection without benefit. A layered design captures the one useful separation here (UI vs business logic vs persistence) more simply, which is easier to defend at the viva. (A second rejected option, a single-file monolith, was dismissed because it would mix SQL, rules and I/O, defeating testability and the parameterised-query guarantee.)

## 2. Component responsibilities

| Layer | Component | Responsibility |
|---|---|---|
| Presentation | Login / Admin / Lecturer / HoD / Student menus | Collect input, display output, call the matching service; no business rules or SQL. |
| Application | AuthService | Authenticate users; enforce role-based access (FR-01/02). |
| Application | UserService, CourseService, AssignmentService | Admin and lecturer management operations (FR-04..FR-09). |
| Application | SubmissionService | Accept submissions/resubmissions; apply the acceptance, replacement and cutoff rules; set lifecycle state (FR-11/12/18/21). |
| Application | MarkingService | Record raw mark and feedback; apply the late penalty via the penalty strategy; write the INITIAL/CHANGE audit event (FR-13/14/17). |
| Application | ModerationService | Department-scoped moderation and controlled release; write MODERATION_CHANGE audit on change (FR-15/16/17). |
| Application | AuditService | Create immutable audit events; read audit history (FR-17/20). |
| Domain | User hierarchy, Course/Assignment, Submission/Mark/AuditEvent, PenaltyPolicy | The entities and business rules (penalty calculation, lifecycle, effective deadline). |
| Data Access | I*Repository + SQLite implementations | CRUD via parameterised queries only; map rows to domain objects (NFR-04). |
| Infrastructure | Database, PasswordHasher (libsodium), Clock | SQLite connection; Argon2id hashing (NFR-03); single time source for deadlines (C6). |

## 3. Key cross-layer rules

- The Presentation layer never touches SQLite; it only calls services.
- Services depend on **repository interfaces**, not on concrete SQLite classes (Dependency Inversion), so the business logic can be tested against in-memory fakes.
- All deadline and lateness comparisons use the single `Clock` source, ensuring consistent timestamps (C5/C6).
- SQLite is embedded (serverless): no database server (e.g. PostgreSQL) is required. The application creates and initialises its own database file at runtime, so the system is built and run with only CMake and a C++17 compiler, and no SQL is executed by hand.
