#!/usr/bin/env python3
"""Validate database/schema.sql against SQLite: schema applies, valid data is
accepted, and every integrity constraint rejects its matching invalid case.
Design verification only - this is NOT the application. Run: python validate_schema.py
"""
import os, sqlite3, sys

HERE = os.path.dirname(os.path.abspath(__file__))
schema = open(os.path.join(HERE, "schema.sql"), encoding="utf-8").read()
c = sqlite3.connect(":memory:")
c.execute("PRAGMA foreign_keys=ON")
c.executescript(schema)
fails = 0

def positive(label, fn):
    global fails
    try:
        fn(); c.commit(); print(f"[PASS]            {label}")
    except Exception as e:
        fails += 1; print(f"[FAIL unexpected] {label}: {e}")

def negative(label, fn):
    global fails
    try:
        fn(); c.commit(); fails += 1; print(f"[FAIL not-rejected] {label}")
    except Exception as e:
        c.rollback(); print(f"[PASS rejected]   {label} ({type(e).__name__})")

def seed():
    c.execute("INSERT INTO departments VALUES (1,'Computer Science')")
    for uid, un, role, dep in [(1,'admin','ADMIN',None),(2,'lec','LECTURER',None),
                               (3,'hod','HOD',1),(4,'stu','STUDENT',None)]:
        c.execute("INSERT INTO users VALUES (?,?,?,?,?,1,?)", (uid,un,'h',un.title(),role,dep))
    c.execute("INSERT INTO courses VALUES (1,'CS101','Intro',1)")
    c.execute("INSERT INTO lecturer_courses VALUES (1,2)")
    c.execute("INSERT INTO enrolments VALUES (1,4)")
    c.execute("INSERT INTO assignments VALUES (1,1,'A1','2026-01-01T00:00:00',100)")
    c.execute("INSERT INTO submissions VALUES (1,1,4,'work','2026-01-01T00:00:00','SUBMITTED')")
    c.execute("INSERT INTO marks VALUES (1,1,80,80,'good work')")
    c.execute("INSERT INTO audit_events VALUES (1,1,'INITIAL',2,'2026-01-02T00:00:00',NULL,80,'Initial mark entry')")

print("SCHEMA APPLIES: OK\n-- positive --")
positive("valid end-to-end seed (incl. INITIAL audit)", seed)
positive("CHANGE with a real reason", lambda: c.execute(
    "INSERT INTO audit_events VALUES (6,1,'CHANGE',2,'t',80,85,'corrected arithmetic error')"))
print("-- negative (must be rejected) --")
negative("invalid role", lambda: c.execute("INSERT INTO users VALUES (9,'x','h','X','SUPERUSER',1,NULL)"))
negative("invalid submission state", lambda: c.execute("INSERT INTO submissions VALUES (3,1,1,'x','t','PENDING')"))
negative("INITIAL with wrong reason", lambda: c.execute("INSERT INTO audit_events VALUES (2,1,'INITIAL',2,'t',NULL,80,'tampered')"))
negative("CHANGE whitespace-only reason", lambda: c.execute("INSERT INTO audit_events VALUES (3,1,'CHANGE',2,'t',80,85,'   ')"))
negative("MODERATION_CHANGE empty reason", lambda: c.execute("INSERT INTO audit_events VALUES (4,1,'MODERATION_CHANGE',3,'t',80,90,'')"))
negative("INITIAL with non-null old_value", lambda: c.execute("INSERT INTO audit_events VALUES (5,1,'INITIAL',2,'t',70,80,'Initial mark entry')"))
negative("UPDATE audit_events (append-only)", lambda: c.execute("UPDATE audit_events SET reason='x' WHERE audit_id=1"))
negative("DELETE audit_events (append-only)", lambda: c.execute("DELETE FROM audit_events WHERE audit_id=1"))
negative("duplicate submission (assignment,student)", lambda: c.execute("INSERT INTO submissions VALUES (2,1,4,'again','t','LATE')"))
negative("assignment max_mark = 0", lambda: c.execute("INSERT INTO assignments VALUES (2,1,'A2','t',0)"))
print(f"\n{'ALL CHECKS PASSED' if fails==0 else str(fails)+' CHECK(S) FAILED'}")
sys.exit(1 if fails else 0)
