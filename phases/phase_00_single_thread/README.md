# Phase 00 — Single Thread

> You have always been here. You just didn't know it had a name.

---

Every program you have ever written runs in at least one thread: the **main thread**.
When the operating system launches your program, it creates a process and starts
executing your `main()` function inside a single thread of execution.

In that world, instructions are **sequential**. The CPU executes one line, finishes
it completely, then moves to the next. There is no ambiguity about order.

```
main()
  │
  ├─ read file        (waits until done)
  ├─ process data     (waits until done)
  ├─ write output     (waits until done)
  └─ return
```

This is not a bad model. It is simple, predictable, and easy to reason about.
But it has a hard limit: **the program can only do one thing at a time**.

---

## The hidden cost of waiting

Consider a program that downloads three files from the network:

```c
download("file_a.txt");   // takes 2 seconds
download("file_b.txt");   // takes 2 seconds
download("file_c.txt");   // takes 2 seconds
// total: 6 seconds
```

Each download spends most of its time *waiting* for the network.
The CPU sits idle while bytes travel across cables.

With multiple threads, all three downloads could happen simultaneously:

```
Thread 1: download("file_a.txt") ─────────── 2s
Thread 2: download("file_b.txt") ─────────── 2s
Thread 3: download("file_c.txt") ─────────── 2s
                                              ^
                                        total: 2s
```

That is the promise of multithreading. The next phases will show you how to
get there — and the problems you will face on the way.

---

## What a thread actually is

A **thread of execution** is a sequence of instructions managed by the kernel.
Every thread in a process shares:

- the same code (text segment)
- the same heap (dynamically allocated memory)
- the same open file descriptors
- the same global and static variables

But each thread has its own:

- **stack** — local variables and function call frames
- **instruction pointer** — where in the code it is currently executing
- **registers** — the CPU state at any given moment
- **thread ID** — a unique identifier (`pthread_t`)

Because threads share the heap and globals, communication between them is
trivially easy. As you will see in Phase 02, this is also where most of the
danger lies.

---

## Threads vs processes

| | Process | Thread |
|---|---------|--------|
| Memory | Separate address space | Shared address space |
| Creation cost | High (full copy of memory) | Low (shares parent memory) |
| Communication | IPC (pipes, sockets, shared mem) | Direct (shared variables) |
| Isolation | Crash doesn't affect others | Crash kills the whole process |

For the Dining Philosophers problem (Phase 09) and most concurrency problems
in systems programming, threads are the right tool.

---

## No new commands in this phase

Phase 00 has no code. Its purpose is to make you aware of the model you have
always been using before introducing the tools to step outside it.

When you are ready, continue to [Phase 01 — Thread Lifecycle](../phase_01_lifecycle/).