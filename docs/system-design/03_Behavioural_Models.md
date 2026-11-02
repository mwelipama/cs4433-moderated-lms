# System Design — Part 3: Behavioural Models

**System:** Moderated Learning Management System · **Author:** Mweli Pama (201404452)

All behaviour follows the accepted requirements: the started-day penalty, the acceptance cutoff, controlled release, and the INITIAL/CHANGE/MODERATION_CHANGE audit model.

## 1. Submission lifecycle — state machine

**Figure 4. Submission Lifecycle State Machine** — [assets/diagrams/state_machine_submission.png](../../assets/diagrams/state_machine_submission.png) · source: [state_machine_submission.puml](../../assets/diagrams/state_machine_submission.puml)

States `Submitted`, `Late`, `Marked`, `Moderated`, `Released` (FR-18). An on-time submission enters `Submitted` and may be replaced by resubmission before D_e (self-transition); a first late submission within the cutoff enters `Late`. Marking moves either state to `Marked` (re-marking is a self-transition that writes a CHANGE audit event). HoD moderation moves `Marked` to `Moderated` (a change writes a MODERATION_CHANGE event; no change writes none). Release moves `Moderated` to `Released`, the terminal state. The note records the cutoff rule: after D_e + 72h no submission is accepted and a missing submission scores zero (FR-21).

## 2. End-to-end process — activity diagram with swimlanes

**Figure 5. Activity Diagram (swimlanes)** — [assets/diagrams/activity_swimlanes.png](../../assets/diagrams/activity_swimlanes.png) · source: [activity_swimlanes.puml](../../assets/diagrams/activity_swimlanes.puml)

Four partitions — Student, System, Lecturer, Head of Department — show the whole path from submission to released mark. The System partition branches on `timestamp ≤ D_e` (on-time vs after-deadline), then on whether **any** submission already exists (refuse replacement, preserving it — an on-time submission is never replaced after D_e, and a late submission is accepted only when none exists) and whether the attempt is within the cutoff (accept as Late, or reject as not submitted). Marking applies the penalty only when the submission is `Late` and writes the INITIAL/CHANGE audit event. Moderation branches on whether the HoD adjusts the mark (MODERATION_CHANGE audit) or accepts it unchanged (Moderated state, no audit). Release is a **partial release** — only submissions already in the `Moderated` state are set to `Released` and made visible; others are untouched — Marking, each moderation action and release are each performed atomically within a single database transaction (see Section 4).

## 3. Sequence diagrams

**Figure 6a. Submit Assignment** — [assets/diagrams/sequence_submit.png](../../assets/diagrams/sequence_submit.png) · source: [sequence_submit.puml](../../assets/diagrams/sequence_submit.puml)

`Student Menu → SubmissionService → Clock, ISubmissionRepository`. The `alt` fragments cover the on-time path (insert or replace, state `Submitted`), the refused replacement of any existing submission (on-time or late) after D_e, a first late submission within the cutoff (state `Late`), and refusal after the cutoff. No penalty is computed here — only lateness is recorded.

**Figure 6b. Moderate and Release Marks** — [assets/diagrams/sequence_moderate_release.png](../../assets/diagrams/sequence_moderate_release.png) · source: [sequence_moderate_release.puml](../../assets/diagrams/sequence_moderate_release.puml)

`HoD Menu → ModerationService → IMarkRepository, AuditService, ISubmissionRepository`. The loop shows the two moderation outcomes — adjust (update mark, write MODERATION_CHANGE, set `Moderated`) versus accept unchanged (set `Moderated`, no audit) — followed by release, which sets `Released` and makes marks visible to students. Each branch and the release are wrapped in `BEGIN TRANSACTION … COMMIT`: the three adjust writes persist together or not at all, an unchanged acceptance still commits its state transition, and release moves the eligible set in one commit with no partial release.

## 4. Transaction boundaries

Every operation that changes a mark, its audit trail or a submission's state is a single ACID transaction. SQLite gives each connection one writer, so the service layer wraps the writes below in `BEGIN … COMMIT` and rolls back on any error. A mark is never saved without its audit event, and no release ever partially commits.

| Operation | Writes performed in one transaction | Atomicity guarantee |
|---|---|---|
| **Marking** (UC-08: initial entry and lecturer re-mark) | save mark + feedback; set submission state to `Marked`; insert the audit event — `INITIAL` (old value NULL, reason "Initial mark entry") on first entry, `CHANGE` (old, new, reason) on a re-mark | the mark/feedback, the state change and the audit event commit together or not at all; a mark can never exist without its audit event, nor an audit event without its mark |
| **Moderation** (UC-12) | *adjust:* update mark; insert `MODERATION_CHANGE` (old, new, reason); set state `Moderated`. *accept unchanged:* set state `Moderated` (no audit event) | the adjust branch's three writes are one unit; an unchanged acceptance still commits its `Moderated` transition, so every moderated submission has a recorded outcome |
| **Release** (UC-13) | set state `Released` for every submission already in `Moderated` state for the assignment | the eligible set is released in one commit; a failure rolls the whole release back, so students never see a partially released cohort; non-`Moderated` submissions are untouched |

On any failure (constraint violation, I/O error, process termination mid-write) the transaction rolls back, leaving the prior consistent state. This is the behavioural counterpart of the append-only audit model in Part 4: `submissions.state` remains the single release authority, and the audit trail is never left inconsistent with the mark it records.
