

This article is explaining the other half of coroutines.

The previous article explained:

co_await
   |
   +--> Awaiter
          |
          +--> await_ready()
          +--> await_suspend()
          +--> await_resume()

This article explains:

Coroutine itself
      |
      V
 Promise Type
Simple Mental Model

Think of a coroutine as having two controllers:

1. Awaiter

Controls a specific co_await

co_await someTask;

The Awaiter decides:

Should I suspend?
When should I resume?
What value should I return?
2. Promise

Controls the entire coroutine.

It decides:

When coroutine starts
What happens on co_return
What happens on exception
What happens at final suspend
What object is returned to caller
Example
Task foo()
{
    co_return 42;
}

You see only:

co_return 42;

But compiler generates much more.

What compiler actually creates

Suppose:

Task foo()
{
    co_return 42;
}

Compiler roughly creates:

struct promise_type
{
    Task get_return_object();

    suspend_never initial_suspend();

    suspend_always final_suspend();

    void return_value(int);

    void unhandled_exception();
};

Then transforms coroutine into:

{
    promise_type promise;

    auto result = promise.get_return_object();

    co_await promise.initial_suspend();

    try
    {
        promise.return_value(42);
    }
    catch(...)
    {
        promise.unhandled_exception();
    }

    co_await promise.final_suspend();
}

This is the BIG idea of the article.

Promise lives inside coroutine frame

When:

Task foo()
{
    co_return 42;
}

is called:

Compiler allocates memory:

Coroutine Frame
+----------------------+
| Parameters           |
| Local variables      |
| Promise Object       |
| State info           |
+----------------------+

The promise object is stored there.

get_return_object()

This is the first important function.

Example:

Task get_return_object()
{
    return Task{
        coroutine_handle<promise_type>::from_promise(*this)
    };
}

Purpose:

Create the object returned to caller

Example:

Task t = foo();

The Task object comes from:

promise.get_return_object()
initial_suspend()

Controls whether coroutine starts immediately.

Example:

std::suspend_never initial_suspend()
{
    return {};
}

Meaning:

Start immediately
std::suspend_always initial_suspend()
{
    return {};
}

Meaning:

Create coroutine
Pause immediately

Execution:

Task t = foo();

Coroutine is created but body does not run yet.

Only when:

t.resume();

does execution begin.

return_value()

When compiler sees:

co_return 42;

it becomes:

promise.return_value(42);

Example:

void return_value(int x)
{
    value = x;
}

Store result in promise.

return_void()

For:

co_return;

Compiler does:

promise.return_void();
unhandled_exception()

If:

Task foo()
{
    throw std::runtime_error("error");
}

Compiler transforms:

try
{
    ...
}
catch(...)
{
    promise.unhandled_exception();
}

Common implementation:

void unhandled_exception()
{
    exception = std::current_exception();
}

Store exception in promise.

Later:

co_await task;

can rethrow it.

final_suspend()

Executed at coroutine end.

co_await promise.final_suspend();

Purpose:

Coroutine completed
Before destruction
Do one final suspend

Typical implementation:

std::suspend_always final_suspend()
{
    return {};
}

Meaning:

Pause before destruction

Now someone can inspect result.

Full Lifecycle

Suppose:

Task foo()
{
    co_return 42;
}

Compiler roughly performs:

1. Allocate coroutine frame

2. Construct promise

3. get_return_object()

4. initial_suspend()

5. Run body

6. return_value(42)

7. Destroy locals

8. final_suspend()

9. Destroy promise

10. Free coroutine frame
Why promise exists?

Without promise:

Compiler wouldn't know:

How to return value?
How to handle exceptions?
Should coroutine start immediately?
Should it suspend at end?
What object should be returned?

Promise answers all these questions.

Relationship between Task and Promise

Most beginners get confused here.

Example:

struct Task
{
    struct promise_type
    {
        ...
    };
};

Looks strange.

Think:

Task
 |
 |--- Object user sees
 |
 Promise
 |
 |--- Object compiler uses

User:

Task t = foo();

Compiler:

promise.get_return_object()
promise.return_value()
promise.final_suspend()
Interview-level summary

A coroutine has two major customization points:

Awaiter

Controls a single co_await.

await_ready()
await_suspend()
await_resume()
Promise

Controls the whole coroutine.

get_return_object()
initial_suspend()
return_value()/return_void()
unhandled_exception()
final_suspend()

A very useful way to remember it is:

Awaiter  -> controls waiting

Promise  -> controls coroutine

or

co_await      => Awaiter

Coroutine     => Promise

Once this distinction is clear, the rest of the coroutine machinery becomes much easier to understand.

for co_await 
get_return_object()
initial_suspend()
return_value()/return_void()
unhandled_exception()
final_suspend()
these function are called or not 


Yes, even if your coroutine contains co_await, those promise functions are still involved.

Think of it this way:

Promise functions
    ↓
Control the entire coroutine

Awaiter functions
    ↓
Control each co_await

For example:

Task foo()
{
    int x = co_await async_op();

    co_return x;
}

The compiler roughly generates:

promise_type promise;

// 1
auto ret = promise.get_return_object();

// 2
co_await promise.initial_suspend();

try
{
    // 3 (your co_await)
    int x = co_await async_op();

    // 4
    promise.return_value(x);
}
catch(...)
{
    // 5
    promise.unhandled_exception();
}

// 6
co_await promise.final_suspend();
What gets called for the co_await async_op()?

For this line:

co_await async_op();

the compiler calls the awaiter functions:

awaiter.await_ready();

awaiter.await_suspend(handle);

awaiter.await_resume();
What gets called for the coroutine itself?

When the coroutine starts and ends, the promise functions are called:

get_return_object()
initial_suspend()
return_value()/return_void()
unhandled_exception()
final_suspend()
Complete timeline

Suppose:

Task foo()
{
    int x = co_await async_op();

    co_return x;
}

Execution:

Call foo()
|
|-- promise constructed
|
|-- get_return_object()
|
|-- initial_suspend()
|
|-- start body
|
|-- co_await async_op()
|      |
|      |-- await_ready()
|      |-- await_suspend()
|      |
|      |  coroutine suspended
|
|------ later resume ------
|
|-- await_resume()
|
|-- return_value(x)
|
|-- final_suspend()
|
|-- destroy coroutine frame
Summary
Function	Who calls it?	When?
get_return_object()	Compiler	Once when coroutine is created
initial_suspend()	Compiler	Before coroutine body starts
await_ready()	Compiler	For each co_await
await_suspend()	Compiler	For each co_await that suspends
await_resume()	Compiler	When that co_await resumes
return_value() / return_void()	Compiler	At co_return
unhandled_exception()	Compiler	If exception escapes coroutine
final_suspend()	Compiler	At coroutine end

So co_await does not call get_return_object(), initial_suspend(), return_value(), etc. directly. Those are part of the coroutine lifecycle. A co_await expression only triggers:

await_ready()
await_suspend()
await_resume()

while the promise functions control the creation, completion, and destruction of the coroutine itself.






/*




This article explains why C++20 introduced symmetric transfer and how it fixes a serious problem in coroutine implementations.

1. The Problem

Consider:

task completes_synchronously() {
    co_return;
}

task loop_synchronously(int count) {
    for(int i = 0; i < count; ++i) {
        co_await completes_synchronously();
    }
}

At first glance this looks like a normal loop.

But internally it behaves like:

loop()
{
    complete();
}

complete()
{
    resume(loop);
}

which becomes:

loop
 └── complete
      └── loop
           └── complete
                └── loop
                     ...

Every resume creates another stack frame.

After enough iterations:

Stack
------
loop
complete
loop
complete
loop
complete
...

Eventually:

Stack Overflow
2. Why does this happen?

Suppose:

co_await foo();

The old implementation did:

await_suspend()
{
    foo_handle.resume();
}

This is just a normal function call.

caller coroutine
    |
    +---- resume(foo)
             |
             +---- foo runs

When foo completes:

final_suspend()
{
    continuation.resume();
}

Again another function call.

caller
  |
  +---- foo
          |
          +---- caller
                  |
                  +---- foo
                          |
                          +---- caller

The stack keeps growing.

3. Original Coroutines TS Fix

People noticed the stack overflow problem.

They used the bool form of await_suspend():

bool await_suspend(...)

Possible returns:

true

means:

Suspend current coroutine
Return to resume() caller

and

false

means:

Immediately resume current coroutine
without adding stack

Example:

bool await_suspend(...)
{
    coro.resume();

    if(completed_synchronously)
        return false;

    return true;
}

This avoided recursion.

4. Why was that solution bad?

It required:

std::atomic<bool> ready;

because completion could happen on another thread.

So every await needed:

atomic exchange

Extra cost:

atomic operations
branches
thread synchronization

Also execution became nondeterministic.

Sometimes continuation resumed on:

Thread A

Sometimes on:

Thread B

depending on who won the race.

5. Symmetric Transfer

C++20 introduced a third form:

std::coroutine_handle<>
await_suspend(...)

Instead of calling:

other.resume();

you simply return the coroutine handle:

return other;
6. What does compiler do?

Compiler roughly generates:

auto h = await_suspend(current);
h.resume();
return;

Notice:

h.resume();
return;

is in tail position.

Meaning:

Call
Return immediately

Compiler converts it into a tail-call.

Instead of:

call h.resume()
return

it becomes:

jmp h.resume()
7. Tail Call

Normal call:

A
 |
 +---- B
        |
        +---- return

Stack:

B
A

Tail call:

A
 |
 +---- jump to B

Stack:

B

A disappears before B starts.

No extra stack usage.

8. Symmetric Transfer Flow
Awaiting

Old:

void await_suspend(...)
{
    coro.resume();
}

New:

std::coroutine_handle<>
await_suspend(...)
{
    return coro;
}
Completion

Old:

void final_suspend(...)
{
    continuation.resume();
}

New:

std::coroutine_handle<>
final_suspend(...)
{
    return continuation;
}
9. What happens now?

Suppose:

loop()
{
    co_await complete();
}

Execution:

loop
   |
   | suspend
   v
complete

The transfer is:

jump

not

call

When complete finishes:

complete
    |
    v
loop

Again:

jump

not

call

So stack remains:

loop
resume

constant size.

Even:

for(int i=0; i<1000000000; ++i)

will not overflow.

10. Why is it called "Symmetric"?

Normal function calls are asymmetric:

A calls B

B returns to A

There is a caller/callee relationship.

Symmetric transfer:

A suspends
B resumes

B suspends
C resumes

C suspends
A resumes

Any coroutine can transfer control to any other coroutine.

There is no fixed caller/callee relationship.

A -> B -> C -> D -> A

all using the same stack depth.

11. The Three Forms of await_suspend()
Form 1
void await_suspend(...)

Always suspend.

Equivalent to:

return std::noop_coroutine();
Form 2
bool await_suspend(...)
true  -> suspend
false -> immediately resume current coroutine

Useful for synchronous completion optimization.

Form 3
std::coroutine_handle<>
await_suspend(...)

Resume another coroutine.

Used for:

task
generator
async frameworks

and enables symmetric transfer.

The Most Important Takeaway

Before symmetric transfer:

co_await
    -> resume()
         -> resume()
              -> resume()
                   -> ...

which could cause:

Stack Overflow

After symmetric transfer:

return coroutine_handle;

Compiler performs a tail-call jump between coroutines.

Result:

Constant stack usage
No atomics
No recursion
Deterministic execution context
Faster coroutine chains

This is why modern coroutine libraries such as cppcoro's task<T> heavily rely on symmetric transfer in await_suspend() and final_suspend().



*/











2:25 PM
Symmetric Transfer: Solving the Stack-Overflow Problem
The core problem: coroutines resuming each other the naive way causes hidden recursion that blows the stack. Symmetric transfer fixes this elegantly.

1. The Problem — Hidden Stack Overflow
Here's innocent-looking code that crashes at high iteration counts:

cpp
task completes_synchronously() {
    co_return;
}

task loop_synchronously(int count) {
    for (int i = 0; i < count; ++i) {
        co_await completes_synchronously();  // looks harmless...
    }
}
Why does it crash? Look at what the naive await_suspend does:

cpp
// NAIVE — void-returning await_suspend
void task::awaiter::await_suspend(std::coroutine_handle<> continuation) {
    coro_.promise().continuation = continuation;
    coro_.resume();  // <-- calls resume(), adds a stack frame
}

// And when the task completes:
void final_awaiter::await_suspend(std::coroutine_handle<promise_type> h) {
    h.promise().continuation.resume();  // <-- calls resume() again!
}
Each loop iteration causes this call chain:

loop_synchronously          (stack frame 1)
  └─ task::awaiter::await_suspend
       └─ coro_.resume()   (stack frame 2)
            └─ completes_synchronously
                 └─ final_awaiter::await_suspend
                      └─ continuation.resume()   (stack frame 3)
                           └─ loop_synchronously  ← BACK HERE, deeper in stack!
After 1,000,000 iterations → stack overflow. The functions are secretly calling each other recursively.

2. The Broken Fix — Atomics (Don't Do This)
The first attempted fix uses bool-returning await_suspend + atomics:

cpp
// Added to promise:
std::atomic<bool> ready = false;

// In awaiter:
bool task::awaiter::await_suspend(std::coroutine_handle<> continuation) {
    promise_type& promise = coro_.promise();
    promise.continuation = continuation;
    coro_.resume();
    // If task already finished, return false (resume immediately without extra frame)
    // If task still running, return true (stay suspended)
    return !promise.ready.exchange(true, std::memory_order_acq_rel);
}

// In final_awaiter:
void final_awaiter::await_suspend(std::coroutine_handle<promise_type> h) {
    promise_type& promise = h.promise();
    // If awaiter already returned true (suspended), we resume it
    // If awaiter hasn't returned yet (still in await_suspend), we don't
    if (promise.ready.exchange(true, std::memory_order_acq_rel)) {
        promise.continuation.resume();
    }
}
This "solves" stack overflow but creates three new problems:

Problem 1: Atomic cost
    Even single-threaded code pays for thread-synchronization overhead
    Two atomic exchanges per co_await — expensive in tight loops

Problem 2: Extra branches
    Caller: "did it finish synchronously? resume or suspend?"
    Callee: "did caller suspend yet? resume or don't?"

Problem 3: Non-deterministic execution context ← most dangerous
    bar1 runs on thread 1234
    foo  runs on thread 1234, then switches to thread 3456
    bar2 might resume on 1234 OR 3456 — you can't know which!
The non-determinism breaks executor-pinning patterns:

cpp
// This is supposed to guarantee execution on thread pool 's'
// but with the atomic fix, it might silently run on the wrong thread
template<typename Awaitable, typename Scheduler>
task<T> via(Awaitable a, Scheduler s) {
    auto result = co_await std::move(a);
    co_await s.schedule();   // move to scheduler s
    co_return result;        // should run on s, but might not!
}
3. The Real Fix — Symmetric Transfer
The key insight: instead of calling .resume() (which adds a stack frame), just return the handle you want to resume.

The compiler turns this into a tail call — it pops the current frame before jumping to the next coroutine. Constant stack depth, no matter how many transfers happen.

cpp
// BEFORE (naive):
void task::awaiter::await_suspend(std::coroutine_handle<> continuation) {
    coro_.promise().continuation = continuation;
    coro_.resume();   // adds stack frame
}

// AFTER (symmetric transfer):
std::coroutine_handle<>                                // ← return a handle
task::awaiter::await_suspend(std::coroutine_handle<> continuation) {
    coro_.promise().continuation = continuation;
    return coro_;     // ← compiler does a tail-call jump, no new frame
}
Same for the completion path:

cpp
// BEFORE:
void final_awaiter::await_suspend(std::coroutine_handle<promise_type> h) {
    h.promise().continuation.resume();  // adds stack frame
}

// AFTER:
std::coroutine_handle<>
final_awaiter::await_suspend(std::coroutine_handle<promise_type> h) {
    return h.promise().continuation;    // tail-call jump back to awaiter
}
4. Visualizing the Stack Difference
NAIVE (stack grows each iteration):

coroutine_handle::resume          ← original caller
  loop_synchronously$resume
    task::awaiter::await_suspend
      coroutine_handle::resume    ← adds frame
        completes_synchronously$resume
          final_awaiter::await_suspend
            coroutine_handle::resume  ← adds frame
              loop_synchronously$resume  ← here again, DEEPER
                ...
                              💥 stack overflow after ~1M iterations


SYMMETRIC TRANSFER (constant stack depth):

coroutine_handle::resume          ← original caller
  loop_synchronously$resume
    [symmetric transfer to completes_synchronously]
  completes_synchronously$resume  ← REPLACES loop frame, doesn't add to it
    [symmetric transfer back to loop_synchronously]
  loop_synchronously$resume       ← REPLACES completes frame
    [symmetric transfer to completes_synchronously]
  ...
                              ✓ constant stack depth forever
5. Complete Working Task Implementation
cpp
#include <coroutine>
#include <utility>
#include <exception>

class task {
public:
    struct promise_type {
        std::coroutine_handle<> continuation;  // who to resume when done

        task get_return_object() noexcept {
            return task{
                std::coroutine_handle<promise_type>::from_promise(*this)
            };
        }

        // Lazy start — don't run until co_awaited
        std::suspend_always initial_suspend() noexcept { return {}; }

        // Final suspend with symmetric transfer back to continuation
        struct final_awaiter {
            bool await_ready() noexcept { return false; }

            // KEY: return the continuation handle instead of calling .resume()
            std::coroutine_handle<>
            await_suspend(std::coroutine_handle<promise_type> h) noexcept {
                auto cont = h.promise().continuation;
                // If no continuation, return noop (go back to resume() caller)
                return cont ? cont : std::noop_coroutine();
            }

            void await_resume() noexcept {}
        };

        final_awaiter final_suspend() noexcept { return {}; }

        void return_void() noexcept {}
        void unhandled_exception() noexcept { std::terminate(); }
    };

    // RAII ownership of the coroutine frame
    task(task&& t) noexcept : coro_(std::exchange(t.coro_, {})) {}
    ~task() { if (coro_) coro_.destroy(); }

    // The awaiter that runs when someone co_awaits this task
    struct awaiter {
        std::coroutine_handle<promise_type> coro_;

        bool await_ready() noexcept { return false; }  // always suspend

        // KEY: return coro_ handle — symmetric transfer to the task
        std::coroutine_handle<>
        await_suspend(std::coroutine_handle<> continuation) noexcept {
            coro_.promise().continuation = continuation;  // save who to resume
            return coro_;   // tail-call jump into the task's coroutine
        }

        void await_resume() noexcept {}
    };

    awaiter operator co_await() && noexcept { return awaiter{coro_}; }

private:
    explicit task(std::coroutine_handle<promise_type> h) noexcept : coro_(h) {}
    std::coroutine_handle<promise_type> coro_;
};

// Now this works for ANY count, no stack overflow:
task completes_synchronously() {
    co_return;
}

task loop_synchronously(int count) {
    for (int i = 0; i < count; ++i) {
        co_await completes_synchronously();  // safe, constant stack depth
    }
}
6. std::noop_coroutine — The Exit Ramp
When you're doing symmetric transfer but have no next coroutine to jump to (you just want to return to whoever called .resume()), use noop_coroutine:

cpp
// noop_coroutine's resume() implementation is literally just: ret
// Returning it means "I'm done, go back to resume() caller"

std::coroutine_handle<> await_suspend(std::coroutine_handle<> h) {
    if (operation_completed_synchronously()) {
        return h;                      // resume current coroutine immediately
    } else {
        enqueue_for_later(h);
        return std::noop_coroutine();  // return to resume() caller
    }
}
This means symmetric transfer is the universal form — it can express everything:

void-returning await_suspend (always suspend, return to caller):
    → return std::noop_coroutine()

bool-returning await_suspend (conditionally resume or suspend):
    → return true  becomes  return std::noop_coroutine()
    → return false becomes  return current_handle

handle-returning await_suspend (resume a specific coroutine):
    → return that_handle
7. When to Use Which Form
┌─────────────────────────────────┬──────────────────────────────────────┐
│ Situation                       │ Use                                  │
├─────────────────────────────────┼──────────────────────────────────────┤
│ Always suspend, return to       │ void await_suspend(handle)           │
│ resume() caller                 │                                      │
├─────────────────────────────────┼──────────────────────────────────────┤
│ Sometimes complete sync,        │ bool await_suspend(handle)           │
│ sometimes suspend               │ (slightly better codegen)            │
├─────────────────────────────────┼──────────────────────────────────────┤
│ Need to resume a DIFFERENT      │ coroutine_handle<> await_suspend()   │
│ coroutine (e.g. task chaining)  │ ← symmetric transfer, use this!     │
└─────────────────────────────────┴──────────────────────────────────────┘
The symmetric transfer form is what makes production-quality coroutine libraries possible — it's what allows co_await-ing tasks in loops to be as safe and efficient as ordinary function calls.

