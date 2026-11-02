# System Design — Part 3: Behavioural Models

**System:** Moderated Learning Management System · **Author:** Mweli Pama (201404452)

All behaviour follows the accepted requirements: the started-day penalty, the acceptance cutoff, controlled release, and the INITIAL/CHANGE/MODERATION_CHANGE audit model.

## 1. Submission lifecycle — state machine

**Figure 4. Submission Lifecycle State Machine** — [assets/diagrams/state_machine_submission.png](../../assets/diagrams/state_machine_submission.png) · source: [state_machine_submission.puml](../../assets/diagrams/state_machine_submission.puml)

States `Submitted`, `Late`, `Marked`, `Moderated`, `Released` (FR-18). An on-time submission enters `Submitted` and may be replaced by resubmission before D_e (self-transition); a first late submission within the cutoff enters `Late`. Marking moves either state to `Marked` (re-marking is a self-transition that writes a CHANGE audit event). HoD moderation moves `Marked` to `Moderated` (a change writes a MODERATION_CHANGE event; no change writes none). Release moves `Moderated` to `Released`, the terminal state. The note records the cutoff rule: after D_e + 72h no submission is accepted and a missing submission scores zero (FR-21).

## 2. End-to-end process — activity diagram with swimlanes

**Figure 5. Activity Diagram (swimlanes)** — [assets/diagrams/activity_swimlanes.png](../../assets/diagrams/activity_swimlanes.png) · source: [activity_swimlanes.puml](../../assets/diagrams/activity_swimlanes.puml)

Four partitions — Student, System, Lecturer, Head of Department — show the whole path from submission to released mark. The System partition branches on `timestamp ≤ D_e` (on-time vs after-deadline), then on whether **any** submission already exists (refuse replacement, preserving it — an on-time submission is never replaced after D_e, and a late submission is accepted only when none exists) and whether the attempt is within the cutoff (accept as Late, or reject as not submitted). Marking applies the penalty only when the submission is `Late` and writes the INITIAL/CHANGE audit event. Moderation branches on whether the HoD adjusts the mark (MODERATION_CHANGE audit) or accepts it unchanged (Moderated state, no audit). Release is a **partial release** — only submissions already in the `Moderated` state are set to `Released` and made visible; others are untouched — and each moderation action (mark update, audit event, state change) is performed atomically within one database transaction.

## 3. Sequence diagrams

**Figure 6a. Submit Assignment** — [assets/diagrams/sequence_submit.png](../../assets/diagrams/sequence_submit.png) · source: [sequence_submit.puml](../../assets/diagrams/sequence_submit.puml)

`Student Menu → SubmissionService → Clock, ISubmissionRepository`. The `alt` fragments cover the on-time path (insert or replace, state `Submitted`), the refused replacement of any existing submission (on-time or late) after D_e, a first late submission within the cutoff (state `Late`), and refusal after the cutoff. No penalty is computed here — only lateness is recorded.

**Figure 6b. Moderate and Release Marks** — [assets/diagrams/sequence_moderate_release.png](../../assets/diagrams/sequence_moderate_release.png) · source: [sequence_moderate_release.puml](../../assets/diagrams/sequence_moderate_release.puml)

`HoD Menu → ModerationService → IMarkRepository, AuditService, ISubmissionRepository`. The loop shows the two moderation outcomes — adjust (update mark, write MODERATION_CHANGE, set `Moderated`) versus accept unchanged (set `Moderated`, no audit) — followed by release, which sets `Released` and makes marks visible to students.
