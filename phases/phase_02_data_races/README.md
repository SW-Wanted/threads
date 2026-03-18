# Phase 02 — Data Races

> The danger of shared memory.

---

## Commands introduced in this phase

These are not Pthreads functions — they are **compiler and tool flags**
used to detect races. There are no new API calls in this phase because
the lesson is about understanding the *problem* before learning the solution.

```bash
# Compile with ThreadSanitizer (TSan) — detects races at runtime
cc -Wall -Wextra -g -fsanitize=thread source.c -lpthread -o program

# Run with Helgrind — Valgrind's race and lock-order detector
valgrind --tool=helgrind ./program

# Run with DRD — another Valgrind race detector
valgrind --tool=drd ./program
```

> Note: `-fsanitize=thread` and `valgrind` cannot be used together.
> Compile with one or the other, not both at the same time.

---

## What is a data race?

A **data race** occurs when two or more threads access the same memory location
simultaneously, at least one of them is writing, and there is no synchronisation
between them.

The result is **undefined behaviour**. The program may produce wrong results,
crash, or appear to work correctly — which is the most dangerous outcome.

---

## A concrete example

```c
static int count = 0;

void *increment(void *arg)
{
    int i = 0;
    while (i < 1000000)
    {
        count++;   // THIS IS NOT ATOMIC
        i++;
    }
    return NULL;
}
```

`count++` looks like one instruction, but the CPU actually performs three steps:

```
1. load   — read the value of count into a register
2. add    — add 1 to the register
3. store  — write the result back to count
```

If two threads interleave these steps, increments are lost:

```
Thread 1: load count (reads 10)
Thread 2: load count (reads 10)   ← also reads 10, not 11
Thread 1: add → 11, store 11
Thread 2: add → 11, store 11      ← should be 12, but both wrote 11
```

Run `ex04.c` to see this happen. The final count will almost certainly
be less than `2 000 000`.

---

## Why is it so hard to detect?

Data races depend on the **exact timing** of the OS scheduler. On a single-core
machine or a lightly loaded system, threads may rarely overlap and the bug stays
hidden. On a multi-core machine under load, it happens constantly.

A program that "works in testing" can fail in production simply because the hardware
is faster or the load is different.

This is why tools like **ThreadSanitizer** and **Helgrind** exist — they
instrument your program to detect concurrent accesses regardless of timing.

---

## How TSan works

When you compile with `-fsanitize=thread`, the compiler inserts instrumentation
around every memory access. At runtime, TSan tracks which thread last wrote to
each memory location and detects when a second thread accesses it without
synchronisation.

The output looks like this:

```
==================
WARNING: ThreadSanitizer: data race (pid=12345)
  Write of size 4 at 0x... by thread T2:
    #0 increment ex04.c:12

  Previous write of size 4 at 0x... by thread T1:
    #0 increment ex04.c:12
==================
```

**TSan has near-zero false positives.** If it reports a race, there is a race.

---

## Exercises

| File | What it demonstrates |
|------|---------------------|
| [ex04.c](ex04.c) | A real data race on a counter — observe the wrong result |
| [ex05.c](ex05.c) | The same race detected by `-fsanitize=thread` |
| [ex06.c](ex06.c) | The same race detected by Helgrind and DRD |

---

## What comes next

The solution to data races is **mutual exclusion**: ensuring that only one thread
at a time can access the shared variable. That is the job of a **mutex**, which
is the topic of Phase 03.