# Phase 05 — Deadlock

> How threads strangle each other — and how to prevent it.

---

## Commands introduced in this phase

No new Pthreads functions are introduced here. Deadlock is not solved by a
new API call — it is solved by **design discipline**.

The tools from Phase 02 remain useful for detecting lock-order violations:

```bash
# Helgrind detects lock-order violations in addition to data races
valgrind --tool=helgrind ./program

# Compile with TSan — also detects some deadlock patterns
cc -g -fsanitize=thread source.c -lpthread -o program
```

---

## What is a deadlock?

A **deadlock** occurs when two or more threads are each waiting for a resource
held by the other, and none can proceed.

The classic scenario with two threads and two mutexes:

```
Thread A                    Thread B
────────────────────        ────────────────────
lock(mutex_1)  ✓            lock(mutex_2)  ✓
lock(mutex_2)  ← BLOCKS     lock(mutex_1)  ← BLOCKS
                     ↑ both waiting forever ↑
```

Thread A holds `mutex_1` and waits for `mutex_2`.
Thread B holds `mutex_2` and waits for `mutex_1`.
Neither can proceed. The program freezes permanently.

A deadlock can also involve a **single thread** that tries to lock a mutex
it already holds (with the default `PTHREAD_MUTEX_NORMAL` type, this deadlocks
the calling thread immediately).

---

## The four conditions for deadlock

All four must be true simultaneously for deadlock to occur:

| Condition | Meaning |
|-----------|---------|
| Mutual exclusion | Resources cannot be shared |
| Hold and wait | A thread holds one resource while waiting for another |
| No preemption | Resources cannot be forcibly taken away |
| Circular wait | Thread A waits for B, B waits for A (or longer chain) |

Breaking **any one** of these conditions prevents deadlock.

---

## Solution 1 — Fixed lock ordering (most common)

Assign a global order to all mutexes and **always acquire them in that order**,
in every thread. This breaks the circular-wait condition.

```c
// WRONG — threads acquire in opposite orders
// Thread A: lock(m1) then lock(m2)
// Thread B: lock(m2) then lock(m1)  ← deadlock possible

// CORRECT — both threads use the same order
// Thread A: lock(m1) then lock(m2)
// Thread B: lock(m1) then lock(m2)  ← no circular wait
```

This is exactly what the Dining Philosophers project does with `assign_forks()`:
even-numbered philosophers pick up the left fork first, odd-numbered pick up
the right fork first — breaking the circular wait.

---

## Solution 2 — Try-and-back-off

Lock the first mutex with `pthread_mutex_lock()`.
Try the second with `pthread_mutex_trylock()`.
If it fails (`EBUSY`), release the first and retry after a short delay.

```c
while (1)
{
    pthread_mutex_lock(&m1);
    if (pthread_mutex_trylock(&m2) == 0)
        break;              // got both — proceed
    pthread_mutex_unlock(&m1);
    usleep(1000);           // back off before retrying
}
// ... critical section using both m1 and m2 ...
pthread_mutex_unlock(&m2);
pthread_mutex_unlock(&m1);
```

This is less efficient than fixed ordering (may retry many times) but more
flexible when a strict order is hard to enforce.

---

## Exercises

| File | What it demonstrates |
|------|---------------------|
| [ex13.c](ex13.c) | A deliberate deadlock — the program hangs |
| [ex14.c](ex14.c) | Fixed with lock ordering — never deadlocks |
| [ex15.c](ex15.c) | Fixed with try-and-back-off |

---

## What to observe

Run `ex13.c` and wait. The program will hang forever. Kill it with `Ctrl+C`.
Then run `ex13.c` under Helgrind — it will detect and report the lock-order
violation before the deadlock even occurs.

```bash
valgrind --tool=helgrind ./ex13
```

Compare with `ex14.c` — identical structure, only the lock order changed.