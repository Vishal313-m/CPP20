/*
Complete Flow

Suppose:

int value = co_await async_read();

Compiler roughly generates:

auto awaiter = async_read().operator co_await();

if (!awaiter.await_ready())
{
    suspend coroutine;

    if (awaiter.await_suspend(handle))
        return;
}

int value = awaiter.await_resume();

Runtime:

Call coroutine
      |
      V
await_ready()
      |
      V
false
      |
      V
Suspend coroutine
      |
      V
await_suspend(handle)
      |
      V
Return to caller

-------------------
Time passes
-------------------

I/O completes

handle.resume()

      |
      V
await_resume()
      |
      V
Continue execution
Why await_suspend() gets the handle

Because somebody must resume the coroutine later.

Example:

void await_suspend(std::coroutine_handle<> h)
{
    start_async_io(
        [h]
        {
            h.resume();
        });
}

The callback stores the handle.

When I/O completes:

h.resume();

Execution continues after the co_await.

What is operator co_await?

The article says:

auto awaiter = get_awaiter(expr);

How does compiler get the awaiter?

It checks:

expr.operator co_await()

or

operator co_await(expr)

If neither exists:

expr

itself is treated as the awaiter.

Example:

struct Task
{
    Awaiter operator co_await();
};

Then:

co_await task;

becomes:

auto awaiter = task.operator co_await();
The key insight of the article

A co_await expression is essentially:

1. Get Awaiter
2. await_ready()
3. Suspend coroutine
4. await_suspend(handle)
5. Return to caller
6. Somebody calls handle.resume()
7. await_resume()
8. Continue execution

Everything else in the coroutine machinery (promise_type, coroutine frame, co_return, initial_suspend, final_suspend) exists mainly to support this pause-and-resume mechanism.

If you understand the flow:

await_ready()
     ↓
await_suspend(handle)
     ↓
handle.resume()
     ↓
await_resume()

then you've understood about 80% of how co_await works internally.

*/





/*
Think of co_await as a person trying to find "the real object that knows how to suspend and resume".

The compiler cannot directly work with your Task object. It needs an object that has:

await_ready()
await_suspend()
await_resume()

That object is called the Awaiter.

Example 1: Task is NOT an Awaiter

Suppose you have:

struct Awaiter
{
    bool await_ready() { return false; }

    void await_suspend(std::coroutine_handle<>) {}

    int await_resume() { return 42; }
};

struct Task
{
    Awaiter operator co_await()
    {
        return Awaiter{};
    }
};

Now write:

int x = co_await task;

Compiler thinks:

"I need an Awaiter object."

So it looks for:

task.operator co_await()

and calls it:

Awaiter awaiter = task.operator co_await();

Now it has an object containing:

await_ready()
await_suspend()
await_resume()

and can continue.

What actually happens?

Your code:

co_await task;

Compiler roughly converts to:

auto awaiter = task.operator co_await();

if (!awaiter.await_ready())
{
    awaiter.await_suspend(handle);
}

awaiter.await_resume();
Why do we need operator co_await()?

Because Task itself may not know how to suspend.

Example:

struct Task
{
    // no await_ready()
    // no await_suspend()
    // no await_resume()
};

This cannot be awaited directly.

So it provides:

Awaiter operator co_await();

which returns an object that does know how.

Think:

Task
 |
 | operator co_await()
 V
Awaiter
 |
 +--> await_ready()
 +--> await_suspend()
 +--> await_resume()
Example 2: Task itself is the Awaiter

You can also do:

struct Task
{
    bool await_ready() { return false; }

    void await_suspend(std::coroutine_handle<>) {}

    int await_resume() { return 42; }
};

Now there is no operator co_await().

Compiler does:

co_await task;

and thinks:

"No operator co_await(). I'll use the object itself."

Equivalent:

auto& awaiter = task;

Then:

awaiter.await_ready();
awaiter.await_suspend(...);
awaiter.await_resume();
Real-world analogy

Imagine co_await task.

The compiler asks:

Compiler: Are you an Awaiter?
Task: No.

Compiler: Do you know someone who is?
Task: Yes, call operator co_await().

Then:

Task
  |
  +---- operator co_await()
                |
                V
             Awaiter

The compiler talks only to the Awaiter from that point on.

The rule to remember

When compiler sees:

co_await expr;

it does:

1. Look for expr.operator co_await()
2. Else look for free operator co_await(expr)
3. Else use expr itself as the Awaiter
4. Call:
      await_ready()
      await_suspend()
      await_resume()

This is the entire purpose of operator co_await():

Convert an object (Task, Future, Event, etc.) into an Awaiter object that knows how to perform the await operation.

*/



2:12 PM
Understanding the C++ Coroutine Promise Type
The Promise type is the control center of a coroutine — it decides what happens at every key lifecycle moment. Think of it less like std::promise and more like a state controller embedded inside the coroutine frame.

1. What the Compiler Actually Generates
When you write a coroutine, the compiler transforms it into roughly this:

cpp
// What YOU write:
Task my_coroutine(int x) {
    co_await something();
    co_return 42;
}

// What the COMPILER generates (roughly):
Task my_coroutine(int x) {
    // 1. Allocate coroutine frame on heap
    auto* frame = new CoroutineFrame(x);

    // 2. Construct promise object inside the frame
    auto& promise = frame->promise;

    // 3. Get the return object BEFORE starting (frame might die later)
    auto returnObject = promise.get_return_object();

    // 4. Initial suspend — should we start immediately or lazily?
    co_await promise.initial_suspend();

    try {
        // 5. YOUR actual code runs here
        co_await something();
        promise.return_value(42);   // co_return 42 becomes this
    } catch (...) {
        promise.unhandled_exception();
    }

    // 6. Final suspend — pause before cleanup?
FinalSuspend:
    co_await promise.final_suspend();
    // frame is destroyed after this
}
Every one of those promise method calls is a customization hook you control.

2. A Complete Promise Type Example
Let's build a Task<T> type with a full promise:

cpp
#include <coroutine>
#include <exception>
#include <stdexcept>
#include <iostream>
#include <optional>

template<typename T>
struct Task {
    // Promise type nested inside Task — compiler finds it here
    struct promise_type {
        std::optional<T>    result;
        std::exception_ptr  exception;

        // ── Called immediately when coroutine is invoked ──
        // Returns the Task object the caller receives
        Task get_return_object() {
            return Task{
                std::coroutine_handle<promise_type>::from_promise(*this)
            };
        }

        // ── Should coroutine start immediately or wait? ──
        // suspend_never  = start executing right away (eager)
        // suspend_always = don't start until someone calls resume() (lazy)
        std::suspend_never initial_suspend() { return {}; }

        // ── Should coroutine pause before being destroyed? ──
        // suspend_always = pause here so caller can read the result
        // suspend_never  = destroy frame immediately (dangerous if result needed)
        std::suspend_always final_suspend() noexcept { return {}; }

        // ── co_return 42; becomes this ──
        void return_value(T value) {
            result = std::move(value);
        }

        // ── Any uncaught exception lands here ──
        void unhandled_exception() {
            exception = std::current_exception(); // save it for later
        }
    };

    std::coroutine_handle<promise_type> handle;

    explicit Task(std::coroutine_handle<promise_type> h) : handle(h) {}

    // RAII: destroy the frame when Task goes out of scope
    ~Task() {
        if (handle) handle.destroy();
    }

    // Get the result (rethrow if there was an exception)
    T get() {
        auto& p = handle.promise();
        if (p.exception)
            std::rethrow_exception(p.exception);
        return *p.result;
    }
};
Usage:

cpp
Task<int> compute() {
    co_return 42;               // calls promise.return_value(42)
}

Task<int> might_fail() {
    throw std::runtime_error("oops");  // calls promise.unhandled_exception()
    co_return 0;
}

int main() {
    auto t = compute();
    std::cout << t.get() << "\n";   // prints 42

    auto t2 = might_fail();
    try {
        t2.get();                   // rethrows the saved exception
    } catch (const std::exception& e) {
        std::cout << e.what() << "\n";  // prints "oops"
    }
}
3. initial_suspend — Eager vs Lazy
This single method controls whether the coroutine starts immediately or waits:

cpp
// EAGER — runs immediately when called, suspends only at co_await
std::suspend_never initial_suspend() { return {}; }

Task<int> eager_task() {
    std::cout << "I run immediately!\n";  // prints right when called
    co_return 1;
}

// LAZY — pauses at start, caller must explicitly resume
std::suspend_always initial_suspend() { return {}; }

Task<int> lazy_task() {
    std::cout << "I only run when resumed!\n";  // caller controls when
    co_return 1;
}

// Lazy tasks are useful for:
// - Coroutines that shouldn't start until resources are ready
// - Generator-style coroutines (produce values on demand)
// - When you want to set up the handle before execution begins
4. final_suspend — The Critical Lifecycle Decision
cpp
// If final_suspend returns suspend_always:
// ✓ Frame stays alive — caller can still read the result
// ✓ Compiler can optimize away heap allocation (lifetime is knowable)
// ✗ You MUST call handle.destroy() manually (or use RAII)

// If final_suspend returns suspend_never:
// ✓ Frame auto-destroys — no manual cleanup needed
// ✗ Result is gone — can't read it after coroutine finishes
// ✗ Harder for compiler to optimize

struct promise_type {
    // Good pattern: suspend at end, let RAII Task destructor clean up
    std::suspend_always final_suspend() noexcept { return {}; }
};
The lifecycle with suspend_always at the end looks like:

call coroutine
    │
    ▼
initial_suspend ──► suspend? ──► resume() called by caller
    │
    ▼
[your coroutine body runs]
    │
    ▼
co_return ──► return_value() or return_void()
    │
    ▼
final_suspend ──► PAUSED HERE (result is safe to read)
    │
    ▼
Task destructor calls handle.destroy() ──► frame freed
5. await_transform — Intercept Every co_await
This is a powerful hook that intercepts every co_await in the coroutine:

cpp
// Use case 1: Make non-awaitable types awaitable
// Coroutine that can co_await on std::optional<T>
template<typename T>
struct optional_promise {
    template<typename U>
    auto await_transform(std::optional<U>& opt) {
        struct awaiter {
            std::optional<U>& opt;

            bool await_ready() { return opt.has_value(); }  // skip suspend if has value
            void await_suspend(std::coroutine_handle<>) {}   // nothing to schedule
            U&   await_resume() { return *opt; }             // unwrap the value
        };
        return awaiter{opt};
    }
};

// Use case 2: Disable co_await entirely (e.g., in a generator)
template<typename T>
struct generator_promise {
    template<typename U>
    std::suspend_never await_transform(U&&) = delete;  // compile error if used
};

// Use case 3: Force resumption on a specific executor/thread
template<typename T>
struct executor_promise {
    Executor& exec;

    template<typename Awaitable>
    auto await_transform(Awaitable&& a) {
        // Wrap every awaitable to resume on our executor
        return resume_on(exec, std::forward<Awaitable>(a));
    }
};
6. co_yield via yield_value
co_yield is just co_await promise.yield_value(expr) under the hood:

cpp
template<typename T>
struct Generator {
    struct promise_type {
        T current_value;

        Generator get_return_object() {
            return Generator{
                std::coroutine_handle<promise_type>::from_promise(*this)
            };
        }

        std::suspend_always initial_suspend() { return {}; }  // lazy start
        std::suspend_always final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() { std::terminate(); }

        // co_yield x  becomes  co_await promise.yield_value(x)
        std::suspend_always yield_value(T value) {
            current_value = std::move(value);  // save the value
            return {};                          // then suspend
        }
    };

    std::coroutine_handle<promise_type> handle;

    // Iterator interface
    bool next() {
        handle.resume();           // run until next co_yield
        return !handle.done();     // false when coroutine finishes
    }

    T value() {
        return handle.promise().current_value;
    }

    ~Generator() { if (handle) handle.destroy(); }
};

// Usage:
Generator<int> count_to(int n) {
    for (int i = 1; i <= n; i++)
        co_yield i;               // suspends here each time, saves i
}

int main() {
    auto gen = count_to(3);
    while (gen.next())
        std::cout << gen.value() << "\n";  // prints 1, 2, 3
}
7. How the Compiler Picks the Promise Type
cpp
// The compiler looks up the promise type like this:
// coroutine_traits<ReturnType, Arg1Type, Arg2Type, ...>::promise_type

// Option A: Nest it inside your return type (most common)
template<typename T>
struct Task {
    using promise_type = TaskPromise<T>;  // compiler finds this
};

// Option B: Specialize coroutine_traits externally
// Useful when you don't own the return type
namespace std {
    template<typename T, typename... Args>
    struct coroutine_traits<std::optional<T>, Args...> {
        using promise_type = OptionalPromise<T>;
    };
}
The Full Picture
Coroutine frame (heap allocated):
┌─────────────────────────────────────┐
│  promise object                     │◄── controls all lifecycle hooks
│    - get_return_object()            │
│    - initial_suspend()              │
│    - return_value() / return_void() │
│    - unhandled_exception()          │
│    - final_suspend()                │
│    - yield_value()     (optional)   │
│    - await_transform() (optional)   │
├─────────────────────────────────────┤
│  local variables                    │
│  parameters (copies)                │
│  resume point (where to jump back)  │
└─────────────────────────────────────┘
         ▲
         │  coroutine_handle points here
         │  — lets you resume() or destroy() from outside
The promise type is what separates a raw coroutine mechanism from a useful abstraction — the same coroutine machinery can power lazy generators, async tasks, optional-unwrapping, and anything else, all by swapping out the promise type.