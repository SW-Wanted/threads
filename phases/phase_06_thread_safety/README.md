# Phase 06 — Thread Safety

> Making functions safe to call from multiple threads simultaneously.

---

## Commands introduced in this phase

```c
#include <pthread.h>

// --- One-time initialisation (run init_func exactly once, ever) ---
pthread_once_t once_var = PTHREAD_ONCE_INIT;
int pthread_once(pthread_once_t *once_control, void (*init_func)(void));

// --- Thread-specific data (per-thread private storage) ---
int   pthread_key_create(pthread_key_t *key, void (*destructor)(void *));
int   pthread_key_delete(pthread_key_t key);
int   pthread_setspecific(pthread_key_t key, const void *value);
void *pthread_getspecific(pthread_key_t key);
```

There is also a compiler extension (not a Pthreads function):

```c
// Thread-local storage — simpler alternative to thread-specific data
// Each thread gets its own copy of the variable.
static __thread char buffer[256];
```

---

## What does "thread-safe" mean?

A function is **thread-safe** if it can be called by multiple threads
simultaneously without producing incorrect results.

A function is **not thread-safe** if it uses global or static state that
is not protected. The classic example is `strerror()`:

```c
// strerror() returns a pointer to a STATIC internal buffer.
// If two threads call it simultaneously, they overwrite each other's result.
char *msg = strerror(errno);   // NOT THREAD-SAFE
```

---

## Three ways to make a function thread-safe

### 1. Mutex — protect the shared state

Add a mutex around every access to the shared variable. Simple, but
serialises concurrent callers (only one thread runs the function at a time).

### 2. Thread-local storage (`__thread`)

Each thread gets its own copy of the variable. No mutex needed.
The simplest solution when the per-thread state does not need to survive
beyond the calling function's scope.

```c
static __thread char buf[256];   // each thread has its own buf
```

### 3. Thread-specific data (Pthreads API)

A more portable and flexible version of thread-local storage.
A single **key** identifies a slot; each thread stores its own pointer
in that slot using `pthread_setspecific()` and retrieves it with
`pthread_getspecific()`.

Useful when the allocated buffer must survive multiple calls to the function
and must be freed when the thread exits (via a destructor).

---

## `pthread_once` — run initialisation exactly once

In a library function, you cannot initialise global state before `main()` is
called (you don't know when the first thread will call your function).
`pthread_once()` guarantees that an init function runs exactly once, regardless
of how many threads call it concurrently:

```c
static pthread_once_t   key_once = PTHREAD_ONCE_INIT;
static pthread_key_t    tsd_key;

static void create_key(void)
{
    pthread_key_create(&tsd_key, free);  // destructor = free
}

void my_library_function(void)
{
    pthread_once(&key_once, create_key);  // safe: runs create_key only once
    // ...
}
```

---

## Exercises

| File | What it demonstrates |
|------|---------------------|
| [ex16.c](ex16.c) | A non-thread-safe function and the bug it causes |
| [ex17.c](ex17.c) | Fixed with `__thread` (thread-local storage) |
| [ex18.c](ex18.c) | Fixed with thread-specific data and `pthread_once` |

When you are ready, continue to [Phase 07 — Cancellation](../phase_07_cancellation/).