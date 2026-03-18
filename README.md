# Fase 0 - Programa em Série

Pthreads data types
The Pthreads API defines a number of data types, some of which are listed in
Table 29-1. We describe most of these data types in the following pages.
Table 29-1: Pthreads data types
Data type Description
pthread_t Thread identifier
pthread_mutex_t Mutex
pthread_mutexattr_t Mutex attributes object
pthread_cond_t Condition variable
pthread_condattr_t Condition variable attributes object
pthread_key_t Key for thread-specific data
pthread_once_t One-time initialization control context
pthread_attr_t Thread attributes object

- pthread_create
- pthread_join
- thread_detach

# Fase 1 — Criação e ciclo de vida de threads

## Thread Creation

```c
int pthread_create(pthread_t *restrict thread,
                   const pthread_attr_t *restrict attr,
                   void *(*start_routine)(void *),
                   void *restrict arg);
```

```c
#include <pthread.h>

int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
 void *(*start)(void *), void *arg);
```

> Returns 0 on success, or a positive error number on error

## Thread Termination

The execution of a thread terminates in one of the following ways:

- The thread’s start function performs a return specifying a return value for the thread.
- The thread calls `pthread_exit()` (described below).
- The thread is canceled using pthread_cancel() (described in Section 32.1).
- Any of the threads calls `exit()`, or the main thread performs a return (in the `main()` function), which causes all threads in the process to terminate immediately.
- The `pthread_exit()` function terminates the calling thread, and specifies a return
  value that can be obtained in another thread by calling `pthread_join()`.

```c
#include <pthread.h>

void pthread_exit(void *retval);
```

Calling pthread_exit() is equivalent to performing a return in the thread’s start function, with the difference that pthread_exit() can be called from any function that has
been called by the thread’s start function.
The retval argument specifies the return value for the thread. The value pointed
to by retval should not be located on the thread’s stack, since the contents of that
stack become undefined on thread termination. (For example, that region of the
process’s virtual memory might be immediately reused by the stack for a new
thread.) The same statement applies to the value given to a return statement in
the thread’s start function.
If the main thread calls pthread_exit() instead of calling exit() or performing a
return, then the other threads continue to execute.

## Thread IDs

Each thread within a process is uniquely identified by a thread ID. This ID is
returned to the caller of pthread_create(), and a thread can obtain its own ID using
pthread_self().

```c
# include <pthread.h>

pthread_t pthread_self(void);
```

> Returns the thread ID of the calling thread

Thread IDs are useful within applications for the following reasons:

- Various Pthreads functions use thread IDs to identify the thread on which they
  are to act. Examples of such functions include `pthread_join()`, `pthread_detach()`,
  `pthread_cancel()`, and `pthread_kill()`, all of which we describe in this and the following chapters.
- In some applications, it can be useful to tag dynamic data structures with the
  ID of a particular thread. This can serve to identify the thread that created or
  “owns” a data structure, or can be used by one thread to identify a specific
  thread that should subsequently do something with that data structure.

The `pthread_equal()` function allows us check whether two thread IDs are the same

```c
#include <pthread.h>

int pthread_equal(pthread_t t1, pthread_t t2);
```

> Returns nonzero value if t1 and t2 are equal, otherwise 0

For example, to check if the ID of the calling thread matches a thread ID saved in
the variable tid, we could write the following:

```c
if (pthread_equal(tid, pthread_self())
    printf("tid matches self\n");
```

## Joining with a Terminated Thread

The `pthread_join()` function waits for the thread identified by thread to terminate. (If that thread has already terminated, `pthread_join() `returns immediately.) This operation is termed joining.

```c
#include <pthread.h>

int pthread_join(pthread_t thread, void **retval);
```

> Returns 0 on success, or a positive error number on error

## Detaching a Thread

By default, a thread is joinable, meaning that when it terminates, another thread can obtain its return status using pthread_join(). Sometimes, we don’t care about the thread’s return status; we simply want the system to automatically clean up and remove the thread when it terminates. In this case, we can mark the thread as detached, by making a call to pthread_detach() specifying the thread’s identifier in thread.

```c
#include <pthread.h>

int pthread_detach(pthread_t thread);
```

> Returns 0 on success, or a positive error number on error

As an example of the use of pthread_detach(), a thread can detach itself using the following call:

```c
pthread_detach(pthread_self());
```

## Thread Attributes

As an example of the use of thread attributes, the code shown in Listing 29-2
creates a new thread that is made detached at the time of thread creation (rather
than subsequently, using `pthread_detach()`). This code first initializes a thread
attributes structure with default values, sets the attribute required to create a
detached thread, and then creates a new thread using the thread attributes structure. Once the thread has been created, the attributes object is no longer needed, and so is destroyed.

```c
thread_t thr;
pthread_attr_t attr;
int s;

s = pthread_attr_init(&attr); /* Assigns default values */
if (s != 0)
    errExitEN(s, "pthread_attr_init");
s = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
if (s != 0)
    errExitEN(s, "pthread_attr_setdetachstate");
s = pthread_create(&thr, &attr, threadFunc, (void *) 1);
if (s != 0)
    errExitEN(s, "pthread_create");
s = pthread_attr_destroy(&attr); /* No longer needed */
if (s != 0)
    errExitEN(s, "pthread_attr_destroy");
```

# Fase 2 — Data races e o problema da memória partilhada

# Fase 3 — Mutex: exclusão mútua

```c
#include <pthread.h>
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);
Both return 0 on success, or a positive error number on error
```

To lock a mutex, we specify the mutex in a call to `pthread_mutex_lock()`. If the mutex
is currently unlocked, this call locks the mutex and returns immediately. If the
mutex is currently locked by another thread, then `pthread_mutex_loc()` blocks until
the mutex is unlocked, at which point it locks the mutex and returns.
If the calling thread itself has already locked the mutex given to
`pthread_mutex_loc()`, then, for the default type of mutex, one of two implementationdefined possibilities may result: the thread deadlocks, blocked trying to lock a
mutex that it already owns, or the call fails, returning the error EDEADLK. On Linux,
the thread deadlocks by default. (We describe some other possible behaviors when
we look at mutex types in Section 30.1.7.)
The `pthread_mutex_unlock()` function unlocks a mutex previously locked by the
calling thread. It is an error to unlock a mutex that is not currently locked, or to
unlock a mutex that is locked by another thread.
If more than one other thread is waiting to acquire the mutex unlocked by a
call to `pthread_mutex_unlock()`, it is indeterminate which thread will succeed in
acquiring it.

`pthread_mutex_trylock()` and `pthread_mutex_timedlock`()`
The` Pthreads API provides two variants of the `pthread_mutex_lock()` function:
`pthread_mutex_trylock()` and `pthread_mutex_timedlock()`. (See the manual pages for
prototypes of these functions.)
The `pthread_mutex_trylock()` function is the same as `pthread_mutex_lock()`, except
that if the mutex is currently locked, `pthread_mutex_trylock()` fails, returning the
error EBUSY.
The `pthread_mutex_timedlock()` function is the same as `pthread_mutex_lock()`,
except that the caller can specify an additional argument, abstime, that places a limit
on the time that the thread will sleep while waiting to acquire the mutex. If the time
interval specified by its abstime argument expires without the caller becoming the
owner of the mutex, `pthread_mutex_timedlock()` returns the error ETIMEDOUT.

The `pthread_mutex_trylock()` and `pthread_mutex_timedlock()` functions are much
less frequently used than `pthread_mutex_lock()`. In most well-designed applications, a
thread should hold a mutex for only a short time, so that other threads are not prevented from executing in parallel. This guarantees that other threads that are
blocked on the mutex will soon be granted a lock on the mutex. A thread that uses
`pthread_mutex_trylock()` to periodically poll the mutex to see if it can be locked risks
being starved of access to the mutex while other queued threads are successively
granted access to the mutex via `pthread_mutex_lock()`.

## Mutex Deadlocks

Sometimes, a thread needs to simultaneously access two or more different shared
resources, each of which is governed by a separate mutex. When more than one
thread is locking the same set of mutexes, deadlock situations can arise. Figure 30-3
shows an example of a deadlock in which each thread successfully locks one mutex,
and then tries to lock the mutex that the other thread has already locked. Both
threads will remain blocked indefinitely.

<!-- Put side by side Threads -->

Thread A

1. pthread_mutex_lock(mutex1);
2. pthread_mutex_lock(mutex2);
   blocks

Thread B

1. pthread_mutex_lock(mutex2);
2. pthread_mutex_lock(mutex1);
   blocks

The simplest way to avoid such deadlocks is to define a mutex hierarchy. When
threads can lock the same set of mutexes, they should always lock them in the same
order. For example, in the scenario in Figure 30-3, the deadlock could be avoided if
the two threads always lock the mutexes in the order mutex1 followed by mutex2.
Sometimes, there is a logically obvious hierarchy of mutexes. However, even if
there isn’t, it may be possible to devise an arbitrary hierarchical order that all
threads should follow.
An alternative strategy that is less frequently used is “try, and then back off.” In this
strategy, a thread locks the first mutex using pthread_mutex_lock(), and then locks the
remaining mutexes using pthread_mutex_trylock(). If any of the pthread_mutex_trylock()
calls fails (with EBUSY), then the thread releases all mutexes, and then tries again,
perhaps after a delay interval. This approach is less efficient than a lock hierarchy,
since multiple iterations may be required. On the other hand, it can be more flexible,
since it doesn’t require a rigid mutex hierarchy. An example of this strategy is
shown in [Butenhof, 1996].

## Initialized a Mutex staticly

```c
#include <pthread.h>

static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
```

## Dynamically Initializing a Mutex

The static initializer value PTHREAD_MUTEX_INITIALIZER can be used only for initializing
a statically allocated mutex with default attributes. In all other cases, we must
dynamically initialize the mutex using pthread_mutex_init().

```c
#include <pthread.h>

int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);
```

> Returns 0 on success, or a positive error number on error

The mutex argument identifies the mutex to be initialized. The attr argument is a
pointer to a pthread_mutexattr_t object that has previously been initialized to define
the attributes for the mutex. (We say some more about mutex attributes in the next
section.) If attr is specified as NULL, then the mutex is assigned various default
attributes.

SUSv3 specifies that initializing an already initialized mutex results in undefined behavior; we should not do this.
Among the cases where we must use pthread_mutex_init() rather than a static
initializer are the following:
z The mutex was dynamically allocated on the heap. For example, suppose that
we create a dynamically allocated linked list of structures, and each structure in
the list includes a pthread_mutex_t field that holds a mutex that is used to protect
access to that structure.
z The mutex is an automatic variable allocated on the stack.
z We want to initialize a statically allocated mutex with attributes other than the
defaults.
When an automatically or dynamically allocated mutex is no longer required, it
should be destroyed using pthread_mutex_destroy(). (It is not necessary to call
pthread_mutex_destroy() on a mutex that was statically initialized using
PTHREAD_MUTEX_INITIALIZER.)

```c
#include <pthread.h>

int pthread_mutex_destroy(pthread_mutex_t *mutex);
```

> Returns 0 on success, or a positive error number on error

It is safe to destroy a mutex only when it is unlocked, and no thread will subsequently try to lock it. If the mutex resides in a region of dynamically allocated memory, then it should be destroyed before freeing that memory region. An
automatically allocated mutex should be destroyed before its host function returns.
A mutex that has been destroyed with `pthread_mutex_destroy()` can subsequently
be reinitialized by `pthread_mutex_init()`.

## Mutex Attributes

As noted earlier, the pthread_mutex_init() attr argument can be used to specify a
`pthread_mutexattr_t` object that defines the attributes of a mutex. Various Pthreads functions can be used to initialize and retrieve the attributes in a `pthread_mutexattr_t`
object. We won’t go into all of the details of mutex attributes or show the prototypes
of the various functions that can be used to initialize the attributes in a
`pthread_mutexattr_t` object. However, we’ll describe one of the attributes that can be
set for a mutex: its type.

## Mutex Types

In the preceding pages, we made a number of statements about the behavior of
mutexes:

- A single thread may not lock the same mutex twice.
- A thread may not unlock a mutex that it doesn’t currently own (i.e., that it did not lock).
- A thread may not unlock a mutex that is not currently locked

Precisely what happens in each of these cases depends on the type of the mutex.
SUSv3 defines the following mutex types:

- PTHREAD_MUTEX_NORMAL
  (Self-)deadlock detection is not provided for this type of mutex. If a thread
  tries to lock a mutex that it has already locked, then deadlock results.
  Unlocking a mutex that is not locked or that is locked by another thread
  produces undefined results. (On Linux, both of these operations succeed
  for this mutex type.)
- PTHREAD_MUTEX_ERRORCHECK
  Error checking is performed on all operations. All three of the above scenarios
  cause the relevant Pthreads function to return an error. This type of mutex
  is typically slower than a normal mutex, but can be useful as a debugging
  tool to discover where an application is violating the rules about how a
  mutex should be used.
- PTHREAD_MUTEX_RECURSIVE
  A recursive mutex maintains the concept of a lock count. When a thread
  first acquires the mutex, the lock count is set to 1. Each subsequent lock
  operation by the same thread increments the lock count, and each unlock
  operation decrements the count. The mutex is released (i.e., made available for other threads to acquire) only when the lock count falls to 0.
  Unlocking an unlocked mutex fails, as does unlocking a mutex that is currently locked by another thread.

The Linux threading implementation provides nonstandard static initializers for
each of the above mutex types (e.g., `PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP`), so that
the use of pthread_mutex_init() is not required to initialize these mutex types for
statically allocated mutexes. However, portable applications should avoid the use
of these initializers.
In addition to the above mutex types, SUSv3 defines the PTHREAD_MUTEX_DEFAULT
type, which is the default type of mutex if we use PTHREAD_MUTEX_INITIALIZER or specify
attr as NULL in a call to pthread_mutex_init(). The behavior of this mutex type is deliberately undefined in all three of the scenarios described at the start of this section,
which allows maximum flexibility for efficient implementation of mutexes. On
Linux, a PTHREAD_MUTEX_DEFAULT mutex behaves like a PTHREAD_MUTEX_NORMAL mutex.
The code shown in Listing 30-3 demonstrates how to set the type of a mutex, in
this case to create an error-checking mutex.

```c
#include <pthread.h>

pthread_mutex_t mtx;
pthread_mutexattr_t mtxAttr;

pthread_mutexattr_init(&mtxAttr);

pthread_mutexattr_settype(&mtxAttr, PTHREAD_MUTEX_ERRORCHECK);

pthread_mutex_init(mtx, &mtxAttr);

pthread_mutexattr_destroy(&mtxAttr); /* No longer needed */
```

## Signaling Changes of State: Condition Variables

A mutex prevents multiple threads from accessing a shared variable at the same
time. A condition variable allows one thread to inform other threads about
changes in the state of a shared variable (or other shared resource) and allows the
other threads to wait (block) for such notification.
A simple example that doesn’t use condition variables serves to demonstrate why
they are useful. Suppose that we have a number of threads that produce some “result
units” that are consumed by the main thread, and that we use a mutex-protected
variable, avail, to represent the number of produced units awaiting consumption:

```c
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
static int avail = 0;
```

In the producer threads, we would have code such as the following:

```c
/* Code to produce a unit omitted */

pthread_mutex_lock(&mtx);
avail++;    /* Let consumer know another unit is available */
pthread_mutex_unlock(&mtx);
```

And in the main (consumer) thread, we could employ the following code:

```c
for (;;) {
    pthread_mutex_lock(&mtx);
    while (avail > 0) { /* Consume all available units */
        /* Do something with produced unit */
        avail--;
    }
    pthread_mutex_unlock(&mtx);
}
```

The above code works, but it wastes CPU time, because the main thread continually loops, checking the state of the variable avail. A condition variable remedies this
problem. It allows a thread to sleep (wait) until another thread notifies (signals) it
that it must do something (i.e., that some “condition” has arisen that the sleeper
must now respond to).
A condition variable is always used in conjunction with a mutex. The mutex
provides mutual exclusion for accessing the shared variable, while the condition
variable is used to signal changes in the variable’s state. (The use of the term signal
here has nothing to do with the signals described in Chapters 20 to 22; rather, it is
used in the sense of indicate.)

## Statically Allocated Condition Variables

As with mutexes, condition variables can be allocated statically or dynamically. We
defer discussion of dynamically allocated condition variables until Section 30.2.5,
and consider statically allocated condition variables here.
A condition variable has the type pthread_cond_t. As with a mutex, a condition
variable must be initialized before use. For a statically allocated condition variable,
this is done by assigning it the value PTHREAD_COND_INITIALIZER, as in the following
example:

```c
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
```

## Signaling and Waiting on Condition Variables

The principal condition variable operations are signal and wait. The signal operation is a notification to one or more waiting threads that a shared variable’s state
has changed. The wait operation is the means of blocking until such a notification
is received.
The pthread_cond_signal() and pthread_cond_broadcast() functions both signal
the condition variable specified by cond. The pthread_cond_wait() function blocks a
thread until the condition variable cond is signaled.

```c
#include <pthread.h>

int pthread_cond_signal(pthread_cond_t *cond);
int pthread_cond_broadcast(pthread_cond_t *cond);
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);
```

> All return 0 on success, or a positive error number on error

The difference between pthread_cond_signal() and pthread_cond_broadcast() lies in
what happens if multiple threads are blocked in pthread_cond_wait(). With
pthread_cond_signal(), we are simply guaranteed that at least one of the blocked
threads is woken up; with pthread_cond_broadcast(), all blocked threads are woken up.
Using pthread_cond_broadcast() always yields correct results (since all threads
should be programmed to handle redundant and spurious wake-ups), but
pthread_cond_signal() can be more efficient. However, pthread_cond_signal() should
be used only if just one of the waiting threads needs to be woken up to handle the
change in state of the shared variable, and it doesn’t matter which one of the waiting threads is woken up. This scenario typically applies when all of the waiting
threads are designed to perform the exactly same task. Given these assumptions,
pthread_cond_signal() can be more efficient than pthread_cond_broadcast(), because it
avoids the following possibility:

1. All waiting threads are awoken.
2. One thread is scheduled first. This thread checks the state of the shared variable(s) (under protection of the associated mutex) and sees that there is work
   to be done. The thread performs the required work, changes the state of the
   shared variable(s) to indicate that the work has been done, and unlocks the
   associated mutex.
3. Each of the remaining threads in turn locks the mutex and tests the state of the
   shared variable. However, because of the change made by the first thread,
   these threads see that there is no work to be done, and so unlock the mutex
   and go back to sleep (i.e., call pthread_cond_wait() once more).
   By contrast, pthread_cond_broadcast() handles the case where the waiting threads are
   designed to perform different tasks (in which case they probably have different
   predicates associated with the condition variable).
   A condition variable holds no state information. It is simply a mechanism for
   communicating information about the application’s state. If no thread is waiting
   on the condition variable at the time that it is signaled, then the signal is lost. A
   thread that later waits on the condition variable will unblock only when the variable
   is signaled once more.
   The pthread_cond_timedwait() function is the same as pthread_cond_wait(),
   except that the abstime argument specifies an upper limit on the time that the
   thread will sleep while waiting for the condition variable to be signaled.

```c
#include <pthread.h>

int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime);
```

> Returns 0 on success, or a positive error number on error

The abstime argument is a timespec structure (Section 23.4.2) specifying an absolute time expressed as seconds and nanoseconds since the Epoch (Section 10.1). If
the time interval specified by abstime expires without the condition variable being
signaled, then pthread_cond_timedwait() returns the error ETIMEDOUT.

Let’s revise our previous example to use a condition variable. The declarations of
our global variable and associated mutex and condition variable are as follows:

```c
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
static int avail = 0;
```

The code in the producer threads is the same as before, except that we add a call to `pthread_cond_signal()`:

```c
pthread_mutex_lock(&mtx);
avail++;    /* Let consumer know another unit is available */
pthread_mutex_unlock(&mtx);
pthread_cond_signal(&cond); /* Wake sleeping consumer */
```

Before considering the code of the consumer, we need to explain
pthread_cond_wait() in greater detail. We noted earlier that a condition variable
always has an associated mutex. Both of these objects are passed as arguments to
pthread_cond_wait(), which performs the following steps:

- unlock the mutex specified by mutex;
- block the calling thread until another thread signals the condition variable cond;
- relock mutex.
-

The `pthread_cond_wait()` function is designed to perform these steps because, normally,
we access a shared variable in the following manner:

```c
pthread_mutex_lock(&mtx);
while (/* Check that shared variable is not in state we want */)
    pthread_cond_wait(&cond, &mtx);

/* Now shared variable is in desired state; do some work */

pthread_mutex_unlock(&mtx);
```

(We explain why the `pthread_cond_wait()` call is placed within a while loop rather
than an if statement in the next section.)
In the above code, both accesses to the shared variable must be mutex-protected
for the reasons that we explained earlier. In other words, there is a natural association of a mutex with a condition variable:

1. The thread locks the mutex in preparation for checking the state of the shared
   variable.
2. The state of the shared variable is checked.
3. If the shared variable is not in the desired state, then the thread must unlock
   the mutex (so that other threads can access the shared variable) before it goes
   to sleep on the condition variable.
4. When the thread is reawakened because the condition variable has been signaled, the mutex must once more be locked, since, typically, the thread then
   immediately accesses the shared variable.
   The `pthread_cond_wait()` function automatically performs the mutex unlocking and
   locking required in the last two of these steps. In the third step, releasing the
   mutex and blocking on the condition variable are performed atomically. In other
   words, it is not possible for some other thread to acquire the mutex and signal
   the condition variable before the thread calling `pthread_cond_wait()` has blocked
   on the condition variable.

Putting the above details together, we can now modify the main (consumer) thread
to use `pthread_cond_wait()`, as follows:

```c
for (;;) {
    pthread_mutex_lock(&mtx);
    while (avail == 0) { /* Wait for something to consume */
        pthread_cond_wait(&cond, &mtx);
    }
    while (avail > 0) { /* Consume all available units */
        /* Do something with produced unit */
        avail--;
    }
    pthread_mutex_unlock(&mtx);
/* Perhaps do other work here that doesn't require mutex lock */
}
```

We conclude with one final observation about the use of pthread_cond_signal()
(and pthread_cond_broadcast()). In the producer code shown earlier, we called
pthread_mutex_unlock(), and then called pthread_cond_signal(); that is, we first unlocked
the mutex associated with the shared variable, and then signaled the corresponding
condition variable. We could have reversed these two steps; SUSv3 permits them to
be done in either order.

## Testing a Condition Variable’s Predicate

Each condition variable has an associated predicate involving one or more shared
variables. For example, in the code segment in the preceding section, the predicate
associated with cond is (avail == 0). This code segment demonstrates a general
design principle: a pthread_cond_wait() call must be governed by a while loop rather
than an if statement. This is so because, on return from pthread_cond_wait(), there
are no guarantees about the state of the predicate; therefore, we should immediately recheck the predicate and resume sleeping if it is not in the desired state.
We can’t make any assumptions about the state of the predicate upon return
from pthread_cond_wait(), for the following reasons:

- Other threads may be woken up first. Perhaps several threads were waiting to
  acquire the mutex associated with the condition variable. Even if the thread
  that signaled the mutex set the predicate to the desired state, it is still possible
  that another thread might acquire the mutex first and change the state of the
  associated shared variable(s), and thus the state of the predicate.
- Designing for “loose” predicates may be simpler. Sometimes, it is easier to design
  applications based on condition variables that indicate possibility rather than
  certainty. In other words, signaling a condition variable would mean “there may
  be something” for the signaled thread to do, rather than “there is something” to
  do. Using this approach, the condition variable can be signaled based on
  approximations of the predicate’s state, and the signaled thread can ascertain
  if there really is something to do by rechecking the predicate.
- Spurious wake-ups can occur. On some implementations, a thread waiting on a
  condition variable may be woken up even though no other thread actually signaled the condition variable. Such spurious wake-ups are a (rare) consequence
  of the techniques required for efficient implementation on some multiprocessor
  systems, and are explicitly permitted by SUSv3.

## Dynamically Allocated Condition Variables

The pthread_cond_init() function is used to dynamically initialize a condition variable. The circumstances in which we need to use pthread_cond_init() are analogous
to those where pthread_mutex_init() is needed to dynamically initialize a mutex
(Section 30.1.5); that is, we must use pthread_cond_init() to initialize automatically
and dynamically allocated condition variables, and to initialize a statically allocated
condition variable with attributes other than the defaults.

```c
#include <pthread.h>

int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr);
```

> Returns 0 on success, or a positive error number on error

The cond argument identifies the condition variable to be initialized. As with
mutexes, we can specify an attr argument that has been previously initialized to
determine attributes for the condition variable. Various Pthreads functions can be
used to initialize the attributes in the pthread_condattr_t object pointed to by attr. If
attr is NULL, a default set of attributes is assigned to the condition variable.

```c
#include <pthread.h>

int pthread_cond_destroy(pthread_cond_t *cond);
```

> Returns 0 on success, or a positive error number on error

It is safe to destroy a condition variable only when no threads are waiting on it. If
the condition variable resides in a region of dynamically allocated memory, then it
should be destroyed before freeing that memory region. An automatically allocated condition variable should be destroyed before its host function returns.
A condition variable that has been destroyed with pthread_cond_destroy() can
subsequently be reinitialized by pthread_cond_init().

## Thread Safety (and Reentrancy Revisited)

A function is said to be thread-safe if it can safely be invoked by multiple threads at
the same time; put conversely, if a function is not thread-safe, then we can’t call it
from one thread while it is being executed in another thread. For example, the following function (similar to code that we looked at in Section 30.1) is not thread-safe:

```c
static int glob = 0;
static void
incr(int loops)
{
 int loc, j;
 for (j = 0; j < loops; j++) {
 loc = glob;
 loc++;
 glob = loc;
 }
}
```

If multiple threads invoke this function concurrently, the final value in glob is
unpredictable. This function illustrates the typical reason that a function is not
thread-safe: it employs global or static variables that are shared by all threads.
There are various methods of rendering a function thread-safe. One way is to
associate a mutex with the function (or perhaps with all of the functions in a
library, if they all share the same global variables), lock that mutex when the function is called, and unlock it when the mutex returns. This approach has the virtue
of simplicity. On the other hand, it means that only one thread at a time can execute the function—we say that access to the function is serialized. If the threads
spend a significant amount of time executing this function, then this serialization
results in a loss of concurrency, because the threads of a program can no longer
execute in parallel.
A more sophisticated solution is to associate the mutex with a shared variable.
We then determine which parts of the function are critical sections that access the
shared variable, and acquire and release the mutex only during the execution of
these critical sections. This allows multiple threads to execute the function at the
same time and to operate in parallel, except when more than one thread needs to
execute a critical section.

### Non-thread-safe functions

To facilitate the development of threaded applications, all of the functions specified in SUSv3 are required to be implemented in a thread-safe manner, except
those listed in Table 31-1. (Many of these functions are not discussed in this book.)
In addition to the functions listed in Table 31-1, SUSv3 specifies the following:
z The ctermid() and tmpnam() functions need not be thread-safe if passed a NULL
argument.
z The wcrtomb() and wcsrtombs() functions need not be thread-safe if their final
argument (ps) is NULL.
SUSv4 modifies the list of functions in Table 31-1 as follows:
z The ecvt(), fcvt(), gcvt(), gethostbyname(), and gethostbyaddr() are removed, since
these functions have been removed from the standard.
z The strsignal() and system() functions are added. The system() function is nonreentrant because the manipulations that it must make to signal dispositions
have a process-wide effect.

### Reentrant and nonreentrant functions

Although the use of critical sections to implement thread safety is a significant
improvement over the use of per-function mutexes, it is still somewhat inefficient
because there is a cost to locking and unlocking a mutex. A reentrant function
achieves thread safety without the use of mutexes. It does this by avoiding the use
of global and static variables. Any information that must be returned to the caller,
or maintained between calls to the function, is stored in buffers allocated by the
caller. (We first encountered reentrancy when discussing the treatment of global
variables within signal handlers in Section 21.1.2.) However, not all functions can
be made reentrant. The usual reasons are the following:
z By their nature, some functions must access global data structures. The functions in the malloc library provide a good example. These functions maintain a
global linked list of free blocks on the heap. The functions of the malloc library
are made thread-safe through the use of mutexes.
z Some functions (defined before the invention of threads) have an interface
that by definition is nonreentrant, because they return pointers to storage statically
allocated by the function, or they employ static storage to maintain information between successive calls to the same (or a related) function. Most of the
functions in Table 31-1 fall into this category. For example, the asctime() function (Section 10.2.3) returns a pointer to a statically allocated buffer containing
a date-time string.

For several of the functions that have nonreentrant interfaces, SUSv3 specifies
reentrant equivalents with names ending with the suffix \_r. These functions require
the caller to allocate a buffer whose address is then passed to the function and used
to return the result. This allows the calling thread to use a local (stack) variable for
the function result buffer. For this purpose, SUSv3 specifies asctime_r(), ctime_r(),
getgrgid_r(), getgrnam_r(), getlogin_r(), getpwnam_r(), getpwuid_r(), gmtime_r(),
localtime_r(), rand_r(), readdir_r(), strerror_r(), strtok_r(), and ttyname_r().

## One-Time Initialization

Sometimes, a threaded application needs to ensure that some initialization action
occurs just once, regardless of how many threads are created. For example, a
mutex may need to be initialized with special attributes using pthread_mutex_init(),
and that initialization must occur just once. If we are creating the threads from the
main program, then this is generally easy to achieve—we perform the initialization
before creating any threads that depend on the initialization. However, in a library
function, this is not possible, because the calling program may create the threads
before the first call to the library function. Therefore, the library function needs a
method of performing the initialization the first time that it is called from any thread.
A library function can perform one-time initialization using the pthread_once()
function.

```c
#include <pthread.h>

int pthread_once(pthread_once_t *once_control, void (*init)(void));
```

The pthread_once() function uses the state of the argument once_control to ensure
that the caller-defined function pointed to by init is called just once, no matter how
many times or from how many different threads the pthread_once() call is made.
The init function is called without any arguments, and thus has the following form:

> Returns 0 on success, or a positive error number on error

```c
void init(void)
{
 /* Function body */
}
```

The once_control argument is a pointer to a variable that must be statically initialized
with the value `PTHREAD_ONCE_INIT`:

```c
pthread_once_t once_var = PTHREAD_ONCE_INIT;
```

The first call to pthread_once() that specifies a pointer to a particular pthread_once_t
variable modifies the value of the variable pointed to by once_control so that subsequent calls to pthread_once() don’t invoke init.
One common use of pthread_once() is in conjunction with thread-specific data,
which we describe next.

## Thread-Specific Data

The most efficient way of making a function thread-safe is to make it reentrant. All
new library functions should be implemented in this way. However, for an existing
nonreentrant library function (one that was perhaps designed before the use of
threads became common), this approach usually requires changing the function’s
interface, which means modifying all of the programs that use the function.
Thread-specific data is a technique for making an existing function thread-safe
without changing its interface. A function that uses thread-specific data may be
slightly less efficient than a reentrant function, but allows us to leave the programs
that call the function unchanged.
Thread-specific data allows a function to maintain a separate copy of a variable
for each thread that calls the function, as illustrated in Figure 31-1. Thread-specific
data is persistent; each thread’s variable continues to exist between the thread’s
invocations of the function. This allows the function to maintain per-thread information between calls to the function, and allows the function to pass distinct result
buffers (if required) to each calling thread.

### Thread-Specific Data from the Library Function’s Perspective

In order to understand the use of the thread-specific data API, we need to consider
things from the point of view of a library function that uses thread-specific data:

- The function must allocate a separate block of storage for each thread that calls
  the function. This block needs to be allocated once, the first time the thread
  calls the function.
- On each subsequent call from the same thread, the function needs to be able
  to obtain the address of the storage block that was allocated the first time this
  thread called the function. The function can’t maintain a pointer to the block
  in an automatic variable, since automatic variables disappear when the function returns; nor can it store the pointer in a static variable, since only one
  instance of each static variable exists in the process. The Pthreads API provides
  functions to handle this task.
- Different (i.e., independent) functions may each need thread-specific data.
  Each function needs a method of identifying its thread-specific data (a key), as
  distinct from the thread-specific data used by other functions.
- The function has no direct control over what happens when the thread terminates. When the thread terminates, it is probably executing code outside the
  function. Nevertheless, there must be some mechanism (a destructor) to ensure
  that the storage block allocated for this thread is automatically deallocated when
  the thread terminates. If this is not done, then a memory leak could occur as
  threads are continuously created, call the function, and then terminate.

### Overview of the Thread-Specific Data API

The general steps that a library function performs in order to use thread-specific
data are as follows:

1. The function creates a key, which is the means of differentiating the thread-specific
   data item used by this function from the thread-specific data items used by
   other functions. The key is created by calling the pthread_key_create() function.
   Creating a key needs to be done only once, when the first thread calls the function.
   For this purpose, pthread_once() is employed. Creating a key doesn’t allocate
   any blocks of thread-specific data.
2. The call to pthread_key_create() serves a second purpose: it allows the caller to
   specify the address of the programmer-defined destructor function that is used
   to deallocate each of the storage blocks allocated for this key (see the next
   step). When a thread that has thread-specific data terminates, the Pthreads API
   automatically invokes the destructor, passing it a pointer to the data block for
   this thread.
3. The function allocates a thread-specific data block for each thread from which
   it is called. This is done using malloc() (or a similar function). This allocation is
   done once for each thread, the first time the thread calls the function.
4. In order to save a pointer to the storage allocated in the previous step, the function employs two Pthreads functions: pthread_setspecific() and pthread_getspecific().
   A call to pthread_setspecific() is a request to the Pthreads implementation to say “save this pointer, recording the fact that it is associated with a particular key (the
   one for this function) and a particular thread (the calling thread).” Calling
   pthread_getspecific() performs the complementary task, returning the pointer previously associated with a given key for the calling thread. If no pointer was previously associated with a particular key and thread, then pthread_getspecific() returns NULL. This is how a function can determine that it is being called for the first time by this thread, and thus must allocate the storage block for the thread.

### Details of the Thread-Specific Data API

In this section, we provide details of each of the functions mentioned in the previous section, and elucidate the operation of thread-specific data by describing how it
is typically implemented. The next section shows how to use thread-specific data to
write a thread-safe implementation of the standard C library function strerror().
Calling pthread_key_create() creates a new thread-specific data key that is
returned to the caller in the buffer pointed to by key.

```c
#include <pthread.h>

int pthread_key_create(pthread_key_t *key, void (*destructor)(void *));
```

> Returns 0 on success, or a positive error number on error

Because the returned key is used by all threads in the process, key should point to a
global variable.
The destructor argument points to a programmer-defined function of the following form:

```c
void dest(void *value)
{
 /* Release storage pointed to by 'value' */
}
```

The pthread_setspecific() function requests the Pthreads API to save a copy of value
in a data structure that associates it with the calling thread and with key, a key
returned by a previous call to pthread_key_create(). The pthread_getspecific() function
performs the converse operation, returning the value that was previously associated with the given key for this thread.

```c
#include <pthread.h>

int pthread_setspecific(pthread_key_t key, const void *value);
```

> Returns 0 on success, or a positive error number on error

```c
void *pthread_getspecific(pthread_key_t key);
```

> Returns pointer, or NULL if no thread-specific data isassociated with key

The value argument given to pthread_setspecific() is normally a pointer to a block of
memory that has previously been allocated by the caller. This pointer will be passed
as the argument for the destructor function for this key when the thread terminates.

When a thread is first created, all of its thread-specific data pointers are initialized
to NULL. This means that when our library function is called by a thread for the first
time, it must begin by using pthread_getspecific() to check whether the thread already
has an associated value for key. If it does not, then the function allocates a block of
memory and saves a pointer to the block using pthread_setspecific(). We show an
example of this in the thread-safe strerror() implementation presented in the next
section

### Thread-Specific Data Implementation Limits

As implied by our description of how thread-specific data is typically implemented,
an implementation may need to impose limits on the number of thread-specific
data keys that it supports. SUSv3 requires that an implementation support at least 128
(\_POSIX_THREAD_KEYS_MAX) keys. An application can determine how many keys an
implementation actually supports either via the definition of PTHREAD_KEYS_MAX
(defined in <limits.h>) or by calling sysconf(\_SC_THREAD_KEYS_MAX). Linux
supports up to 1024 keys.
Even 128 keys should be more than sufficient for most applications. This is
because each library function should employ only a small number of keys—often
just one. If a function requires multiple thread-specific data values, these can usually
be placed in a single structure that has just one associated thread-specific data key.

## Thread-Local Storage

Like thread-specific data, thread-local storage provides persistent per-thread storage. This feature is nonstandard, but it is provided in the same or a similar form on
many other UNIX implementations (e.g., Solaris and FreeBSD).
The main advantage of thread-local storage is that it is much simpler to use
than thread-specific data. To create a thread-local variable, we simply include the
**thread specifier in the declaration of a global or static variable:
static **thread buf[MAX_ERROR_LEN];
Each thread has its own copy of the variables declared with this specifier. The variables in a thread’s thread-local storage persist until the thread terminates, at which
time the storage is automatically deallocated.
Note the following points about the declaration and use of thread-local variables:

- The \_\_thread keyword must immediately follow the static or extern keyword, if
  either of these is specified in the variable’s declaration.
- The declaration of a thread-local variable can include an initializer, in the same
  manner as a normal global or static variable declaration.
- The C address (&) operator can be used to obtain the address of a thread-local
  variable.
  Thread-local storage requires support from the kernel (provided in Linux 2.6), the
  Pthreads implementation (provided in NPTL), and the C compiler (provided on
  x86-32 with gcc 3.3 and later).
  Listing 31-4 shows a thread-safe implementation of strerror() using thread-local
  storage. If we compile and link our test program (Listing 31-2) with this version of
  strerror() to create an executable file, strerror_test_tls, then we see the following
  results when running the program:

```bash
$ ./strerror_test_tls
Main thread has called strerror()
Other thread about to call strerror()
Other thread: str (0x40376ab0) = Operation not permitted
Main thread: str (0x40175080) = Invalid argument
```

## Canceling a Thread

The `pthread_cancel()` function sends a cancellation request to the specified thread.

```c
#include <pthread.h>

int pthread_cancel(pthread_t thread);
```

> Returns 0 on success, or a positive error number on error
> Having made the cancellation request, pthread_cancel() returns immediately; that is,
> it doesn’t wait for the target thread to terminate.
> Precisely what happens to the target thread, and when it happens, depends on
> that thread’s cancellation state and type, as described in the next section.

## Cancellation State and Type

The `pthread_setcancelstate()` and `pthread_setcanceltype()` functions set flags that allow a
thread to control how it responds to a cancellation request.

```c
#include <pthread.h>
int pthread_setcancelstate(int state, int *oldstate);
int pthread_setcanceltype(int type, int *oldtype);
```

> Both return 0 on success, or a positive error number on error

The pthread_setcancelstate() function sets the calling thread’s cancelability state to
the value given in state. This argument has one of the following values:

_PTHREAD_CANCEL_DISABLE_
The thread is not cancelable. If a cancellation request is received, it
remains pending until cancelability is enabled.

_PTHREAD_CANCEL_ENABLE_
The thread is cancelable. This is the default cancelability state in newly created threads.
The thread’s previous cancelability state is returned in the location pointed to by
oldstate.

Temporarily disabling cancellation (PTHREAD_CANCEL_DISABLE) is useful if a thread is
executing a section of code where all of the steps must be completed.
If a thread is cancelable (PTHREAD_CANCEL_ENABLE), then the treatment of a cancellation request is determined by the thread’s cancelability type, which is specified by
the type argument in a call to pthread_setcanceltype(). This argument has one of the
following values:

_PTHREAD_CANCEL_ASYNCHRONOUS_
The thread may be canceled at any time (perhaps, but not necessarily,
immediately). Asynchronous cancelability is rarely useful, and we defer
discussion of it until Section 32.6.

_PTHREAD_CANCEL_DEFERRED_
The cancellation remains pending until a cancellation point (see the next
section) is reached. This is the default cancelability type in newly created
threads. We say more about deferred cancelability in the following sections.

The thread’s previous cancelability type is returned in the location pointed to by
oldtype.

As with the pthread_setcancelstate() oldstate argument, many implementations,
including Linux, allow oldtype to be specified as NULL if we are not interested in
the previous cancelability type. Again, SUSv3 doesn’t specify this feature, and
portable applications can’t rely on it We should always specify a non-NULL value
for oldtype.

When a thread calls fork(), the child inherits the calling thread’s cancelability type and
state. When a thread calls exec(), the cancelability type and state of the main thread of
the new program are reset to PTHREAD_CANCEL_ENABLE and PTHREAD_CANCEL_DEFERRED,
respectively.

## Cancellation Points

When cancelability is enabled and deferred, a cancellation request is acted upon
only when a thread next reaches a cancellation point. A cancellation point is a call to
one of a set of functions defined by the implementation.
SUSv3 specifies that the functions shown in Table 32-1 must be cancellation
points if they are provided by an implementation. Most of these are functions that
are capable of blocking the thread for an indefinite period of time.
Upon receiving a cancellation request, a thread whose cancelability is enabled and
deferred terminates when it next reaches a cancellation point. If the thread was not
detached, then some other thread in the process must join with it, in order to prevent it
from becoming a zombie thread. When a canceled thread is joined, the value
returned in the second argument to pthread_join() is a special thread return value:
PTHREAD_CANCELED.

Listing 32-1 shows a simple example of the use of pthread_cancel(). The main program creates a thread that executes an infinite loop, sleeping for a second and
printing the value of a loop counter. (This thread will terminate only if it is sent a
cancellation request or if the process exits.) Meanwhile, the main program sleeps
for 3 seconds, and then sends a cancellation request to the thread that it created.
When we run this program, we see the following:

```bash
$ ./t_pthread_cancel
New thread started
Loop 1
Loop 2
Loop 3
Thread was canceled
```

> Canceling a thread with pthread_cancel()

```c
#include <pthread.h>

static void * threadFunc(void *arg)
{
 int j;
 printf("New thread started\n"); /* May be a cancellation point */
 for (j = 1; ; j++) {
 printf("Loop %d\n", j); /* May be a cancellation point */
 sleep(1); /* A cancellation point */
 }
 /* NOTREACHED */
 return NULL;
}

int main(int argc, char *argv[])
{
 pthread_t thr;
 void *res;

 pthread_create(&thr, NULL, threadFunc, NULL);
 sleep(3); /* Allow new thread to run a while */
 pthread_cancel(thr);
 pthread_join(thr, &res);
 if (res == PTHREAD_CANCELED)
    printf("Thread was canceled\n");
 else
    printf("Thread was not canceled (should not happen!)\n");
 exit(EXIT_SUCCESS);
}
```

## Testing for Thread Cancellation

In Listing 32-1, the thread created by main() accepted the cancellation request
because it executed a function that was a cancellation point (sleep() is a cancellation
point; printf() may be one). However, suppose a thread executes a loop that contains no cancellation points (e.g., a compute-bound loop). In this case, the thread
would never honor the cancellation request.

The purpose of pthread_testcancel() is simply to be a cancellation point. If a cancellation is pending when this function is called, then the calling thread is terminated.

```c
#include <pthread.h>

void pthread_testcancel(void);
```

A thread that is executing code that does not otherwise include cancellation points
can periodically call pthread_testcancel() to ensure that it responds in a timely fashion to a cancellation request sent by another thread.

## Cleanup Handlers

If a thread with a pending cancellation were simply terminated when it reached a
cancellation point, then shared variables and Pthreads objects (e.g., mutexes)
might be left in an inconsistent state, perhaps causing the remaining threads in the
process to produce incorrect results, deadlock, or crash. To get around this problem,
a thread can establish one or more cleanup handlers—functions that are automatically executed if the thread is canceled. A cleanup handler can perform tasks such
as modifying the values of global variables and unlocking mutexes before the
thread is terminated.

Each thread can have a stack of cleanup handlers. When a thread is canceled,
the cleanup handlers are executed working down from the top of the stack; that is,
the most recently established handler is called first, then the next most recently
established, and so on. When all of the cleanup handlers have been executed, the
thread terminates.

The pthread_cleanup_push() and pthread_cleanup_pop() functions respectively
add and remove handlers on the calling thread’s stack of cleanup handlers.

```c
#include <pthread.h>

void pthread_cleanup_push(void (*routine)(void*), void *arg);
void pthread_cleanup_pop(int execute);
```

The pthread_cleanup_push() function adds the function whose address is specified
in routine to the top of the calling thread’s stack of cleanup handlers. The routine
argument is a pointer to a function that has the following form:

```c
void routine(void *arg)
{
 /* Code to perform cleanup */
}
```

The arg value given to pthread_cleanup_push() is passed as the argument of the
cleanup handler when it is invoked. This argument is typed as void \*, but, using
judicious casting, other data types can be passed in this argument.
Typically, a cleanup action is needed only if a thread is canceled during the execution of a particular section of code. If the thread reaches the end of that section
without being canceled, then the cleanup action is no longer required. Thus, each

call to pthread_cleanup_push() has an accompanying call to pthread_cleanup_pop().
This function removes the topmost function from the stack of cleanup handlers. If
the execute argument is nonzero, the handler is also executed. This is convenient if
we want to perform the cleanup action even if the thread was not canceled.
Although we have described pthread_cleanup_push() and pthread_cleanup_pop()
as functions, SUSv3 permits them to be implemented as macros that expand to
statement sequences that include an opening ({) and closing (}) brace, respectively.
Not all UNIX implementations do things this way, but Linux and many others do.
This means that each use of pthread_cleanup_push() must be paired with exactly one
corresponding pthread_cleanup_pop() in the same lexical block. (On implementations that do things this way, variables declared between the pthread_cleanup_push()
and pthread_cleanup_pop() will be limited to that lexical scope.) For example, it is
not correct to write code such as the following:

```c
pthread_cleanup_push(func, arg);
...
if (cond) {
 pthread_cleanup_pop(0);
}
```

As a coding convenience, any cleanup handlers that have not been popped are also
executed automatically if a thread terminates by calling pthread_exit() (but not if it
does a simple return).

```bash
$ ./thread_cleanup
thread: allocated memory at 0x804b050
main: about to cancel thread
cleanup: freeing block at 0x804b050
cleanup: unlocking mutex
main: thread was canceled
```

```bash
$ ./thread_cleanup s
thread: allocated memory at 0x804b050
main: about to signal condition variable
thread: condition wait loop completed
cleanup: freeing block at 0x804b050
cleanup: unlocking mutex
main: thread terminated normally
```

## Asynchronous Cancelability

When a thread is made asynchronously cancelable (cancelability type
PTHREAD_CANCEL_ASYNCHRONOUS), it may be canceled at any time (i.e., at any machinelanguage instruction); delivery of a cancellation is not held off until the thread next
reaches a cancellation point.
The problem with asynchronous cancellation is that, although cleanup handlers
are still invoked, the handlers have no way of determining the state of a thread. In
the program in Listing 32-2, which employs the deferred cancelability type, the
thread can be canceled only when it executes the call to pthread_cond_wait(), which
is the only cancellation point. By this time, we know that buf has been initialized to
point to a block of allocated memory and that the mutex mtx has been locked. However, with asynchronous cancelability, the thread could be canceled at any point;
for example, before the malloc() call, between the malloc() call and locking the
mutex, or after locking the mutex. The cleanup handler has no way of knowing
where cancellation has occurred, or precisely which cleanup steps are required.
Furthermore, the thread might even be canceled during the malloc() call, after
which chaos is likely to result (Section 7.1.3).
As a general principle, an asynchronously cancelable thread can’t allocate any
resources or acquire any mutexes, semaphores, or locks. This precludes the use of a
wide range of library functions, including most of the Pthreads functions. (SUSv3
makes exceptions for pthread_cancel(), pthread_setcancelstate(), and pthread_setcanceltype(),
which are explicitly required to be async-cancel-safe; that is, an implementation must
make them safe to call from a thread that is asynchronously cancelable.) In other
words, there are few circumstances where asynchronous cancellation is useful. One
such circumstance is canceling a thread that is in a compute-bound loop

## Thread Stacks

Each thread has its own stack whose size is fixed when the thread is created. On
Linux/x86-32, for all threads other than the main thread, the default size of the
per-thread stack is 2 MB. (On some 64-bit architectures, the default size is higher;
for example, it is 32 MB on IA-64.) The main thread has a much larger space for
stack growth (refer to Figure 29-1, on page 618).
Occasionally, it is useful to change the size of a thread’s stack. The
pthread_attr_setstacksize() function sets a thread attribute (Section 29.8) that determines the size of the stack in threads created using the thread attributes object. The
related pthread_attr_setstack() function can be used to control both the size and the
location of the stack, but setting the location of a stack can decrease application
portability. The manual pages provide details of these functions.
One reason to change the size of per-thread stacks is to allow for larger stacks
for threads that allocate large automatic variables or make nested function calls of great depth (perhaps because of recursion). Alternatively, an application may want
to reduce the size of per-thread stacks to allow for a greater number of threads
within a process. For example, on x86-32, where the user-accessible virtual address
space is 3 GB, the default stack size of 2 MB means that we can create a maximum
of around 1500 threads. (The precise maximum depends on how much virtual
memory is consumed by the text and data segments, shared libraries, and so on.)
The minimum stack that can be employed on a particular architecture can be
determined by calling sysconf(\_SC_THREAD_STACK_MIN). For the NPTL implementation on Linux/x86-32, this call returns the value 16,384.

## Threads and Signals

The UNIX signal model was designed with the UNIX process model in mind, and
predated the arrival of Pthreads by a couple of decades. As a result, there are some
significant conflicts between the signal and thread models. These conflicts arose
primarily from the need to maintain the traditional signal semantics for singlethreaded processes (i.e., the signal semantics of traditional programs should not be
changed by Pthreads), while at the same time developing a signal model that would
be usable within a multithreaded process.
The differences between the signal and thread models mean that combining
signals and threads is complex, and should be avoided whenever possible. Nevertheless, sometimes we must deal with signals in a threaded program. In this section,
we discuss the interactions between threads and signals, and describe various functions
that are useful in threaded programs that deal with signals.

### Manipulating the Thread Signal Mask

When a new thread is created, it inherits a copy of the signal mask of the thread
that created it. A thread can use pthread_sigmask() to change its signal mask, to retrieve
the existing mask, or both.

```c
#include <signal.h>

int pthread_sigmask(int how, const sigset_t *set, sigset_t *oldset);
```

> Returns 0 on success, or a positive error number on error

Other than the fact that it operates on the thread signal mask, the use of
pthread_sigmask() is the same as the use of sigprocmask().

### Sending a Signal to a Thread

The pthread_kill() function sends the signal sig to another thread in the same process.
The target thread is identified by the argument thread.

```c
#include <signal.h>

int pthread_kill(pthread_t thread, int sig);
```

> Returns 0 on success, or a positive error number on error

Because a thread ID is guaranteed to be unique only within a process (see
Section 29.5), we can’t use pthread_kill() to send a signal to a thread in another
process.

```c
#define _GNU_SOURCE
#include <signal.h>

int pthread_sigqueue(pthread_t thread, int sig, const union sigval value);
```

> Returns 0 on success, or a positive error number on erro

### Dealing with Asynchronous Signals Sanely

In Chapters 20 to 22, we discussed various factors—such as reentrancy issues, the
need to restart interrupted system calls, and avoiding race conditions—that can make
it complex to deal with asynchronously generated signals via signal handlers. Furthermore, none of the functions in the Pthreads API is among the set of async-signal-safe
functions that we can safely call from within a signal handler (Section 21.1.2). For
these reasons, multithreaded programs that must deal with asynchronously generated signals generally should not use a signal handler as the mechanism to receive
notification of signal delivery. Instead, the preferred approach is the following:
z All threads block all of the asynchronous signals that the process might receive.
The simplest way to do this is to block the signals in the main thread before any
other threads are created. Each subsequently created thread will inherit a copy
of the main thread’s signal mask.
z Create a single dedicated thread that accepts incoming signals using sigwaitinfo(),
sigtimedwait(), or sigwait(). We described sigwaitinfo() and sigtimedwait() in Section 22.10. We describe sigwait() below.
The advantage of this approach is that asynchronously generated signals are
received synchronously. As it accepts incoming signals, the dedicated thread can
safely modify shared variables (under mutex control) and call non-async-signal-safe
functions. It can also signal condition variables, and employ other thread and process communication and synchronization mechanisms.
The sigwait() function waits for the delivery of one of the signals in the signal
set pointed to by set, accepts that signal, and returns it in sig.

```c
#include <signal.h>

int sigwait(const sigset_t *set, int *sig);
```

> Returns 0 on success, or a positive error number on error

The operation of sigwait() is the same as sigwaitinfo(), except that:
z instead of returning a siginfo_t structure describing the signal, sigwait() returns
just the signal number; and
z the return value is consistent with other thread-related functions (rather than
the 0 or –1 returned by traditional UNIX system calls).
If multiple threads are waiting for the same signal with sigwait(), only one of the
threads will actually accept the signal when it arrives. Which of the threads this will
be is indeterminate

## Threads and Process Control

Like the signals mechanism, exec(), fork(), and exit() predate the Pthreads API. In the
following paragraphs, we note some details concerning the use of these system calls
in threaded programs.
Threads and exec()
When any thread calls one of the exec() functions, the calling program is completely
replaced. All threads, except the one that called exec(), vanish immediately. None of
the threads executes destructors for thread-specific data or calls cleanup handlers.
All of the (process-private) mutexes and condition variables belonging to the process
also disappear. After an exec(), the thread ID of the remaining thread is unspecified.
Threads and fork()
When a multithreaded process calls fork(), only the calling thread is replicated in
the child process. (The ID of the thread in the child is the same as the ID of the
thread that called fork() in the parent.) All of the other threads vanish in the child;
no thread-specific data destructors or cleanup handlers are executed for those
threads. This can lead to various problems:
z Although only the calling thread is replicated in the child, the states of global
variables, as well as all Pthreads objects such as mutexes and condition variables, are preserved in the child. (This is so because these Pthreads objects are
allocated within the parent’s memory, and the child gets a duplicate of that
memory.) This can lead to tricky scenarios. For example, suppose that another
thread had locked a mutex at the time of the fork() and is part-way through
updating a global data structure. In this case, the thread in the child would not
be able to unlock the mutex (since it is not the mutex owner) and would block
if it tried to acquire the mutex. Furthermore, the child’s copy of the global data
structure is probably in an inconsistent state, because the thread that was
updating it vanished part-way through the update.
z Since destructors for thread-specific data and cleanup handlers are not called,
a fork() in a multithreaded program can cause memory leaks in the child. Furthermore, the thread-specific data items created by other threads are likely to
be inaccessible to the thread in the new child, since it doesn’t have pointers
referring to these items.
Threads: Further Details 687
Because of these problems, the usual recommendation is that the only use of fork()
in a multithreaded process should be one that is followed by an immediate exec().
The exec() causes all of the Pthreads objects in the child process to disappear as the
new program overwrites the memory of the process.
For programs that must use a fork() that is not followed by an exec(), the
Pthreads API provides a mechanism for defining fork handlers. Fork handlers are
established using a pthread_atfork() call of the following form:
pthread_atfork(prepare_func, parent_func, child_func);
Each pthread_atfork() call adds prepare_func to a list of functions that will be automatically executed (in reverse order of registration) before the new child process is
created when fork() is called. Similarly, parent_func and child_func are added to a list
functions that will be called automatically (in order of registration), in, respectively,
the parent and child process, just before fork() returns.
Fork handlers are sometimes useful for library code that makes use of threads.
In the absence of fork handlers, there would be no way for the library to deal with
applications that naively make use of the library and call fork(), unaware that the
library has created some threads.
The child produced by fork() inherits fork handlers from the thread that called
fork(). During an exec(), fork handlers are not preserved (they can’t be, since the
code of the handlers is overwritten during the exec()).
Further details on fork handlers, and examples of their use, can be found in
[Butenhof, 1996].
On Linux, fork handlers are not called if a program using the NPTL threading
library calls vfork(). However, in a program using LinuxThreads, fork handlers
are called in this case.
Threads and exit()
If any thread calls exit() or, equivalently, the main thread does a return, all threads
immediately vanish; no thread-specific data destructors or cleanup handlers are
executed.

## Thread Implementation Models

In this section, we go into some theory, briefly considering three different models
for implementing a threading API. This provides useful background for Section 33.5,
where we consider the Linux threading implementations. The differences between
these implementation models hinge on how threads are mapped onto kernel
scheduling entities (KSEs), which are the units to which the kernel allocates the CPU
and other system resources. (In traditional UNIX implementations that predate
threads, the term kernel scheduling entity is synonymous with the term process.)

### Many-to-one (M:1) implementations (user-level threads)

In M:1 threading implementations, all of the details of thread creation, scheduling,
and synchronization (mutex locking, waiting on condition variables, and so on) are
handled entirely within the process by a user-space threading library. The kernel
knows nothing about the existence of multiple threads within the process.
M:1 implementations have a few advantages. The greatest advantage is that
many threading operations—for example, creating and terminating a thread, context
switching between threads, and mutex and condition variable operations—are fast,
since a switch to kernel mode is not required. Furthermore, since kernel support
for the threading library is not required, an M:1 implementation can be relatively
easily ported from one system to another.
However, M:1 implementations suffer from some serious disadvantages:
z When a thread makes a system call such as read(), control passes from the userspace threading library to the kernel. This means that if the read() call blocks,
then all threads in the process are blocked.
z The kernel can’t schedule the threads of a process. Since the kernel is unaware
of the existence of multiple threads within the process, it can’t schedule the
separate threads to different processors on multiprocessor hardware. Nor is it
possible to meaningfully assign a thread in one process a higher priority than a
thread in another process, since the scheduling of the threads is handled
entirely within the process.

### One-to-one (1:1) implementations (kernel-level threads)

In a 1:1 threading implementation, each thread maps onto a separate KSE. The
kernel handles each thread’s scheduling separately. Thread synchronization operations are implemented using system calls into the kernel.
1:1 implementations eliminate the disadvantages suffered by M:1 implementations. A blocking system call does not cause all of the threads in a process to block,
and the kernel can schedule the threads of a process onto different CPUs on multiprocessor hardware.
However, operations such as thread creation, context switching, and synchronization are slower on a 1:1 implementations, since a switch into kernel mode is
required. Furthermore, the overhead required to maintain a separate KSE for each
of the threads in an application that contains a large number of threads may place a
significant load on the kernel scheduler, degrading overall system performance.
Despite these disadvantages, a 1:1 implementation is usually preferred over an
M:1 implementation. Both of the Linux threading implementations—LinuxThreads
and NPTL—employ the 1:1 model.

### Many-to-many (M:N) implementations (two-level model)

M:N implementations aim to combine the advantages of the 1:1 and M:1 models,
while eliminating their disadvantages.
In the M:N model, each process can have multiple associated KSEs, and several
threads may map to each KSE. This design permits the kernel to distribute the
threads of an application across multiple CPUs, while eliminating the possible scaling problems associated with applications that employ large numbers of threads.

## Linux Implementations of POSIX Threads

Linux has two main implementations of the Pthreads API:

- LinuxThreads: This is the original Linux threading implementation, developed
  by Xavier Leroy.
- NPTL (Native POSIX Threads Library): This is the modern Linux threading
  implementation, developed by Ulrich Drepper and Ingo Molnar as a successor
  to LinuxThreads. NPTL provides performance that is superior to LinuxThreads,
  and it adheres more closely to the SUSv3 specification for Pthreads. Support
  for NPTL required changes to the kernel, and these changes appeared in
  Linux 2.6.

In the following sections, we consider further details of these two implementations,
and note the points where they deviate from the SUSv3 requirements for Pthreads.
At this point, it is worth emphasizing that the LinuxThreads implementation is
now obsolete; it is not supported in glibc 2.4 and later. All new thread library development occurs only in NPTL.

# Fase 4 — Deadlock e ordem de aquisição

# Fase 5 — Tempo e sincronização temporal

# Fase 6 — Flags partilhadas e estado global seguro
