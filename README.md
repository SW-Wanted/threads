# Threads in C

> From sequential thinking to mastering concurrency with POSIX threads.

Most programmers spend years writing code that runs in a single thread without
ever realising it. Every `for` loop waits for the previous iteration. Every function
call waits for the callee to return. Every `read()` freezes the entire program.

This repository takes you from that world — one instruction at a time — to the
full POSIX threads model: parallelism, synchronisation, safety, cancellation, and
the classic problems that emerge when multiple threads share the same memory.

The content is based on the threads chapters of
**The Linux Programming Interface** by Michael Kerrisk (No Starch Press).

---

## Prerequisites

| Tool | Purpose |
|------|---------|
| `gcc` or `cc` | Compiler |
| `make` | Build system |
| `valgrind` | Detect races and deadlocks |
| Linux | POSIX threads (`-lpthread`) |

```bash
sudo apt install build-essential valgrind
```

---

## How to use this repository

```bash
git clone https://github.com/SW-Wanted/threads
cd threads
```

| Command | What it does |
|---------|-------------|
| `make` | Compile every exercise across all phases |
| `make phase=01` | Compile only the exercises in phase 01 |
| `make run ex=phases/phase_01_lifecycle/ex01` | Compile and immediately run one exercise |
| `make clean` | Remove all compiled binaries |
| `make list` | Print all phases and their exercise files |

To compile and run a single exercise manually:
```bash
cc -Wall -Wextra -Werror -g -fsanitize=thread phases/phase_01_lifecycle/ex01.c -lpthread -o ex01
./ex01
```

> **Note:** `-fsanitize=thread` detects data races at runtime automatically.
> Do not use it together with `valgrind` — compile without the flag for Helgrind/DRD runs.

---

## Phases

| # | Phase | What you will learn |
|---|-------|---------------------|
| [00](phases/phase_00_single_thread/) | Single thread | You have always lived here without knowing it |
| [01](phases/phase_01_lifecycle/) | Thread lifecycle | `pthread_create`, `pthread_join`, `pthread_detach`, `pthread_self`, `pthread_exit` |
| [02](phases/phase_02_data_races/) | Data races | The danger of shared memory, TSan, Helgrind |
| [03](phases/phase_03_mutex/) | Mutex | Mutual exclusion, lock patterns, mutex types |
| [04](phases/phase_04_condition_variables/) | Condition variables | Waiting for *something to happen*, not just for a lock |
| [05](phases/phase_05_deadlock/) | Deadlock | How threads strangle each other — and how to prevent it |
| [06](phases/phase_06_thread_safety/) | Thread safety | Reentrancy, `pthread_once`, thread-specific data |
| [07](phases/phase_07_cancellation/) | Cancellation | `pthread_cancel`, cleanup handlers, cancellation points |
| [08](phases/phase_08_advanced/) | Advanced | Thread attributes, signals, `fork()` in threaded programs |

---

## Reading order

Read each phase's `README.md` before looking at the `.c` files.
The README explains the *why*. The code shows the *how*.

Every phase (except phase 00) opens with a **Commands** section listing every
new function introduced in that phase. Use it as a reference card.

---

## Data types introduced by the Pthreads API

Before diving in, it helps to know what types you will encounter:

| Type | Description |
|------|-------------|
| `pthread_t` | Thread identifier |
| `pthread_mutex_t` | Mutex |
| `pthread_mutexattr_t` | Mutex attributes object |
| `pthread_cond_t` | Condition variable |
| `pthread_condattr_t` | Condition variable attributes object |
| `pthread_key_t` | Key for thread-specific data |
| `pthread_once_t` | One-time initialisation control context |
| `pthread_attr_t` | Thread attributes object |

These types are defined in `<pthread.h>`.
Always compile with `-lpthread` on Linux.
