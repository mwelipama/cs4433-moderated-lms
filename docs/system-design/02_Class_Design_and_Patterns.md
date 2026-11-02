# System Design — Part 2: Class Design, Design Patterns and SOLID

**System:** Moderated Learning Management System · **Author:** Mweli Pama (201404452)

**Figure 3. Design-level Class Diagram** — [assets/diagrams/class_diagram.png](../../assets/diagrams/class_diagram.png) · source: [class_diagram.puml](../../assets/diagrams/class_diagram.puml)

## 1. Domain model overview

An abstract `User` holds the common account state (id, username, password hash, display name, active flag) and a pure-virtual `role()`; `Administrator`, `Lecturer`, `HeadOfDepartment` (adds `departmentId`) and `Student` specialise it — satisfying the required abstract-`User`-with-inheritance-and-polymorphism design. `Department`, `Course`, `Assignment` and `Extension` model the academic structure; `Submission` carries the lifecycle `state` and the behaviour `effectiveDeadline()` and `daysLate()`; `Mark` holds the raw and awarded marks, feedback and the released flag; `AuditEvent` records each mark-affecting action. Enumerations `Role`, `SubmissionState` and `AuditEventType` are first-class types in the design and map to constrained `TEXT` columns in the schema.

## 2. Design patterns (justified)

**Pattern 1 — Strategy (`PenaltyPolicy`).** The late-penalty rule is encapsulated behind a `PenaltyPolicy` interface with a `StandardPenaltyPolicy` implementing `A = clamp(R − 0.10·M·d, 0, M)`. `MarkingService` depends on the interface. *Why:* the penalty is the most likely rule to change (a different school policy), and isolating it keeps that change out of the marking workflow; it is also directly unit-testable with boundary cases (d = 0, 1, 3, > 3). This is the pattern the brief suggests for late-penalty rules.

**Pattern 2 — Repository / DAO (`I*Repository`).** Each aggregate has a repository interface (`IUserRepository`, `ISubmissionRepository`, `IMarkRepository`, `IAuditRepository`, …) with a SQLite implementation (`Sqlite*Repository`). *Why:* it concentrates all SQL — and therefore the **parameterised-query** guarantee (NFR-04) — in one layer, maps rows to domain objects, and lets the services be tested against in-memory fakes (supporting the 70% business-logic coverage target). This is the pattern the brief suggests for data access.

**Pattern 3 — Factory (`UserFactory`).** `UserFactory.create(role, data)` returns the correct `User` subclass for a role. *Why:* account creation happens in several places (Admin seeding, login hydration); the factory keeps the role→subclass mapping in one place and preserves the polymorphic `User` type, so the rest of the system never switches on role to build a user.

## 3. SOLID application

- **Single Responsibility:** each service owns one workflow (marking, moderation, submission); each repository owns one aggregate's persistence; the menus own only I/O.
- **Open/Closed:** a new penalty rule is a new `PenaltyPolicy` implementation — `MarkingService` is not modified (the Strategy pattern realises this).
- **Liskov Substitution:** every `User` subclass is usable wherever `User` is expected (authentication, display, audit acting-user); no subclass weakens a base contract.
- **Interface Segregation:** repository interfaces are small and aggregate-specific, so a service depends only on the operations it uses rather than one large data interface.
- **Dependency Inversion:** services depend on repository and `PenaltyPolicy` **interfaces**; concrete SQLite classes and the concrete policy are injected, so the business logic is independent of SQLite and testable in isolation.
