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

*   pthread_create
*   pthread_join
*   thread_detach

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
* The thread’s start function performs a return specifying a return value for the thread.
* The thread calls `pthread_exit()` (described below).
* The thread is canceled using pthread_cancel() (described in Section 32.1).
* Any of the threads calls `exit()`, or the main thread performs a return (in the `main()` function), which causes all threads in the process to terminate immediately.
* The `pthread_exit()` function terminates the calling thread, and specifies a return
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

* Various Pthreads functions use thread IDs to identify the thread on which they
are to act. Examples of such functions include `pthread_join()`, `pthread_detach()`,
`pthread_cancel()`, and `pthread_kill()`, all of which we describe in this and the following chapters.
* In some applications, it can be useful to tag dynamic data structures with the
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

# Fase 4 — Deadlock e ordem de aquisição

# Fase 5 — Tempo e sincronização temporal

# Fase 6 — Flags partilhadas e estado global seguro


