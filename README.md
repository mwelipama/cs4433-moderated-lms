# CS4433 — Moderated Learning Management System

Software Engineering individual project — a Moderated Learning Management System
for a secondary school, developed with **C++17** and **SQLite**.

- **Course:** CS4433 Software Engineering — National University of Lesotho
- **Author:** Mweli Pama (201404452)

## Scope

Four roles — Administrator, Lecturer, Head of Department, Student — with
role-based access control, assignment submission and resubmission, individual
extensions, late-penalty rules, Head-of-Department moderation and controlled
mark release, over persistent SQLite storage with a mark-change audit trail.

Out of scope: online payments, video conferencing, automated grading,
plagiarism detection, and web/mobile interfaces.

## Current status

- **Phase 1 — Project Planning & Management: complete.** Feasibility study
  (technical, operational, economic, legal, schedule), Work Breakdown Structure,
  a seven-week schedule with dependencies and critical path, risk register, and
  phase-gate governance. See the planning documents in this repository.
- **Phases 2–8 — planned, not yet implemented:** Requirements Engineering,
  System Design, Implementation (C++17/SQLite), Verification & Validation,
  Configuration Management & QA, Deployment & Maintenance, and Professional/
  Legal/Ethical analysis.

This README describes only work that currently exists; it does not claim any
implemented application functionality yet.

## Contents (Phase 1)

- `CS4433 Final Project -Individual Assignment.pdf` — assignment brief
- `CS4433_Phase1_Project_Management_Plan_Mweli_Pama_201404452.docx` / `.pdf` — Phase 1 report
- `CS4433_Phase1_Project_Schedule_Mweli_Pama_201404452.xlsx` — schedule & Gantt
- `CS4433_Phase1_Project_Schedule_..._Appendix_1.pdf` / `Appendix_2.pdf` — schedule appendices

## Planned repository structure

As later phases are built: `src/`, `include/`, `tests/`, `database/`,
`docs/` (project-management, requirements, system-design, testing, deployment,
maintenance, professional-practice, final-report), `assets/`, and
`.github/workflows/`.
