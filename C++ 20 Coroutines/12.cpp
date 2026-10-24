
/* https://lewissbaker.github.io/2022/08/27/understanding-the-compiler-transform */


/*  https://quantdev.blog/posts/coroutines/ */



Explain each everthing with code with proper code with proper explanation

2:27 PM
The Complete Compiler Transform: How Coroutines Actually Work
This is the deepest dive — showing exactly what the compiler generates for a coroutine. Let's walk through every piece.

The Coroutine We're Transforming
cpp
task f(int x);  // some async function

task g(int x) {
    int fx = co_await f(x);
    co_return fx * fx;
}
The compiler transforms this into four pieces:

A coroutine state struct (the heap-allocated frame)
A ramp function (initializes everything, returns to caller)
A resume function (the state machine)
A destroy function (cleanup at any suspend point)
Part 0: Foundation Types
Before anything, we need helper types the compiler uses internally.

The Coroutine State Base
cpp
// Every coroutine frame starts with this — it's what coroutine_handle points to.
// The two function pointers are what resume() and destroy() call.
struct __coroutine_state {
    // Returns the NEXT state to resume (for symmetric transfer / trampoline)
    using __resume_fn  = __coroutine_state*(__coroutine_state*);
    using __destroy_fn = void(__coroutine_state*);

    __resume_fn*  __resume;   // null = at final suspend point (done())
    __destroy_fn* __destroy;

    // Special singleton: "resume me" = do nothing, return to caller
    static __coroutine_state* __noop_resume(__coroutine_state* s) noexcept {
        return s;  // returning itself signals "stop the trampoline"
    }
    static void __noop_destroy(__coroutine_state*) noexcept {}

    static const __coroutine_state __noop_coroutine;
};

inline const __coroutine_state __coroutine_state::__noop_coroutine{
    &__coroutine_state::__noop_resume,
    &__coroutine_state::__noop_destroy
};
The Promise Base (so from_promise() works)
cpp
// Separating the promise into a base class gives it a consistent
// memory offset from the start of any coroutine frame — needed for
// coroutine_handle<Promise>::from_promise() to work correctly.
template<typename Promise>
struct __coroutine_state_with_promise : __coroutine_state {
    __coroutine_state_with_promise() noexcept {}
    ~__coroutine_state_with_promise() {}

    // Stored in a union so the DERIVED class controls when it's constructed
    // (promise constructor needs the parameter copies to exist first)
    union { Promise __promise; };
};
manual_lifetime<T> — Explicit Lifetime Control
cpp
// Objects in the coroutine frame can't use normal constructors/destructors
// because their lifetimes are manually controlled across suspend points.
// This wrapper reserves aligned storage without constructing anything.
template<typename T>
struct manual_lifetime {
    manual_lifetime() noexcept = default;
    ~manual_lifetime() = default;

    // Not copyable/movable — it's raw storage
    manual_lifetime(const manual_lifetime&) = delete;
    manual_lifetime& operator=(const manual_lifetime&) = delete;

    // Construct in-place using a factory lambda (enables guaranteed copy elision)
    template<typename Factory>
    T& construct_from(Factory factory) noexcept(std::is_nothrow_invocable_v<Factory>) {
        return *::new(static_cast<void*>(&storage)) T(factory());
    }

    void destroy() noexcept(std::is_nothrow_destructible_v<T>) {
        std::destroy_at(std::launder(reinterpret_cast<T*>(&storage)));
    }

    T& get() noexcept {
        return *std::launder(reinterpret_cast<T*>(&storage));
    }

private:
    alignas(T) std::byte storage[sizeof(T)];
};

// Trivially destructible types don't need manual destroy — specialize it away
template<typename T> requires std::is_trivially_destructible_v<T>
struct manual_lifetime<T> {
    manual_lifetime() noexcept = default;
    ~manual_lifetime() = default;
    manual_lifetime(const manual_lifetime&) = delete;
    manual_lifetime& operator=(const manual_lifetime&) = delete;

    template<typename Factory>
    T& construct_from(Factory factory) noexcept(std::is_nothrow_invocable_v<Factory>) {
        return *::new(static_cast<void*>(&storage)) T(factory());
    }

    void destroy() noexcept {}  // no-op for trivially destructible types

    T& get() noexcept {
        return *std::launder(reinterpret_cast<T*>(&storage));
    }

private:
    alignas(T) std::byte storage[sizeof(T)];
};
destructor_guard<T> — RAII for Coroutine Frame Members
cpp
// When an exception is thrown inside the coroutine body, objects stored
// in the coroutine frame (manual_lifetime members) won't auto-destruct.
// This RAII guard ensures they get destroyed on scope exit.
template<typename T>
struct destructor_guard {
    explicit destructor_guard(manual_lifetime<T>& obj) noexcept
        : ptr_(std::addressof(obj)) {}

    destructor_guard(destructor_guard&&) = delete;
    destructor_guard& operator=(destructor_guard&&) = delete;

    ~destructor_guard() noexcept(std::is_nothrow_destructible_v<T>) {
        if (ptr_) ptr_->destroy();
    }

    // Call this when the coroutine SUSPENDS — objects must stay alive
    void cancel() noexcept { ptr_ = nullptr; }

private:
    manual_lifetime<T>* ptr_;
};

// Trivially destructible types don't need the guard at all
template<typename T> requires std::is_trivially_destructible_v<T>
struct destructor_guard<T> {
    explicit destructor_guard(manual_lifetime<T>&) noexcept {}
    void cancel() noexcept {}
};

template<typename T>
destructor_guard(manual_lifetime<T>&) -> destructor_guard<T>;
Part 1: coroutine_handle Implementation
cpp
namespace std {

template<typename Promise = void>
class coroutine_handle;

// Type-erased handle — can point to ANY coroutine
template<>
class coroutine_handle<void> {
public:
    coroutine_handle() noexcept = default;

    void* address() const noexcept {
        return static_cast<void*>(state_);
    }

    static coroutine_handle from_address(void* ptr) noexcept {
        coroutine_handle h;
        h.state_ = static_cast<__coroutine_state*>(ptr);
        return h;
    }

    explicit operator bool() const noexcept { return state_ != nullptr; }

    // THE KEY: trampoline loop — keeps resuming coroutines until noop
    void resume() const {
        __coroutine_state* s = state_;
        do {
            s = s->__resume(s);         // each resume returns the NEXT state
        } while (s != &__coroutine_state::__noop_coroutine); // stop at noop
    }

    void destroy() const { state_->__destroy(state_); }

    // null __resume means we're at the final suspend point
    bool done() const { return state_->__resume == nullptr; }

private:
    __coroutine_state* state_ = nullptr;
};

// Promise-specific handle — adds promise() and from_promise()
template<typename Promise>
class coroutine_handle : public coroutine_handle<void> {
    using state_t = __coroutine_state_with_promise<Promise>;
public:
    using coroutine_handle<void>::coroutine_handle;

    Promise& promise() const {
        return static_cast<state_t*>(
            static_cast<__coroutine_state*>(/* state_ */ nullptr)
        )->__promise;
        // simplified — in practice accesses the base state_ field
    }

    // Reconstruct handle FROM a promise reference
    // Works by subtracting the offset of __promise from its address
    static coroutine_handle from_promise(Promise& promise) noexcept {
        coroutine_handle h;
        // Calculate coroutine state address from promise address
        auto* state = reinterpret_cast<state_t*>(
            reinterpret_cast<unsigned char*>(std::addressof(promise)) -
            offsetof(state_t, __promise)
        );
        // set state_ via base class... (simplified here)
        return h;
    }
};

// The noop coroutine — "return to resume() caller"
struct noop_coroutine_promise {};
using noop_coroutine_handle = coroutine_handle<noop_coroutine_promise>;

noop_coroutine_handle noop_coroutine() noexcept { return {}; }

template<>
class coroutine_handle<noop_coroutine_promise> {
public:
    operator coroutine_handle<void>() const noexcept {
        return coroutine_handle<void>::from_address(address());
    }

    // These are all no-ops — resuming the noop coroutine does nothing
    void resume() const noexcept {}
    void destroy() const noexcept {}
    bool done() const noexcept { return false; }

    // Points to the global noop singleton — trampoline loop checks for this
    void* address() const noexcept {
        return const_cast<__coroutine_state*>(&__coroutine_state::__noop_coroutine);
    }
};

} // namespace std
Part 2: The Coroutine State Struct
This is the heap-allocated frame. It holds everything that must survive across suspension points.

cpp
using __g_promise_t = std::coroutine_traits<task, int>::promise_type;
// Resolves to task::promise_type

// Forward declarations — defined below
__coroutine_state* __g_resume(__coroutine_state* s);
void __g_destroy(__coroutine_state* s);

struct __g_state : __coroutine_state_with_promise<__g_promise_t> {

    // Constructor: copies parameters, initialises promise
    __g_state(int&& __x)
    : x(static_cast<int&&>(__x))   // 1. copy the parameter
    {
        // 2. Set up the resume/destroy function pointers
        this->__resume  = &__g_resume;
        this->__destroy = &__g_destroy;

        // 3. Construct promise AFTER parameter copies
        //    (promise constructor can accept lvalue refs to params)
        ::new(static_cast<void*>(std::addressof(this->__promise)))
            __g_promise_t(construct_promise<__g_promise_t>(x));
    }

    ~__g_state() {
        this->__promise.~__g_promise_t();  // must manually destroy (union member)
    }

    // ── Suspend-point tracker ──────────────────────────────────────────────
    int __suspend_point = 0;  // 0=initial, 1=after co_await f(x), 2=final

    // ── Parameter copies ───────────────────────────────────────────────────
    int x;

    // ── Temporaries whose lifetimes span suspend points ────────────────────
    //
    // Lifetimes:                           __tmp1     __s1      __tmp4
    //   initial_suspend() result:          [=====]
    //   f(x) task + awaiter:                         [=====]
    //   final_suspend() result:                                 [=====]
    //
    // __tmp1, __tmp4, and __s1 never overlap → put them in a UNION
    // (saves memory — they share the same storage)
    struct __scope1 {
        manual_lifetime<task>          __tmp2;  // the task returned by f(x)
        manual_lifetime<task::awaiter> __tmp3;  // the awaiter for that task
    };

    union {
        manual_lifetime<std::suspend_always>              __tmp1;  // initial_suspend
        __scope1                                          __s1;    // co_await f(x)
        manual_lifetime<task::promise_type::final_awaiter> __tmp4; // final_suspend
    };
};
Memory layout visualization:

__g_state (on heap):
┌─────────────────────────────────────┐
│  __resume   (fn ptr)                │  ← base class __coroutine_state
│  __destroy  (fn ptr)                │
├─────────────────────────────────────┤
│  __promise  (task::promise_type)    │  ← base class __coroutine_state_with_promise
├─────────────────────────────────────┤
│  __suspend_point  (int)             │  ← which suspend point we're at
│  x                (int)             │  ← parameter copy
├─────────────────────────────────────┤
│  ┌──────────────────────────────┐   │
│  │ UNION (shared storage):      │   │
│  │  __tmp1  (suspend_always)    │   │  ← used during initial_suspend
│  │  __s1.__tmp2 (task)          │   │  ← used during co_await f(x)
│  │  __s1.__tmp3 (task::awaiter) │   │
│  │  __tmp4  (final_awaiter)     │   │  ← used during final_suspend
│  └──────────────────────────────┘   │
└─────────────────────────────────────┘
Part 3: The Ramp Function
This is the function with the original signature. It initializes everything and handles the initial suspend.

cpp
task g(int x) {
    // ── Step 1: Allocate coroutine frame on heap ───────────────────────────
    // unique_ptr ensures cleanup if anything throws before we release it
    std::unique_ptr<__g_state> state(new __g_state(static_cast<int&&>(x)));

    // ── Step 2: Get the return object BEFORE starting ──────────────────────
    // Critical: the frame might be destroyed before resume() returns,
    // so we must capture the return object now.
    decltype(auto) return_value = state->__promise.get_return_object();

    // ── Step 3: Evaluate co_await promise.initial_suspend() ───────────────
    // initial_suspend() returns std::suspend_always (stored in __tmp1)
    state->__tmp1.construct_from([&]() -> decltype(auto) {
        return state->__promise.initial_suspend();
    });

    if (!state->__tmp1.get().await_ready()) {
        // Coroutine WILL suspend at the initial point
        // await_suspend() for suspend_always is a no-op returning void
        state->__tmp1.get().await_suspend(
            std::coroutine_handle<__g_promise_t>::from_promise(state->__promise)
        );

        // Release ownership — frame now lives until someone calls resume/destroy
        state.release();

        // Return to caller. Coroutine is suspended at suspend_point = 0.
    } else {
        // Coroutine doesn't suspend initially — start executing body immediately
        __g_resume(state.release());
    }

    return return_value;
}
// After this returns, the caller has the task object and the coroutine
// is sitting suspended at suspend_point 0, waiting to be resumed.
Part 4: The Resume Function (The State Machine)
This is the heart of the whole system — a goto-based state machine.

cpp
__coroutine_state* __g_resume(__coroutine_state* s) {
    auto* state = static_cast<__g_state*>(s);

    // ── Jump to wherever we last suspended ────────────────────────────────
    // (NOTE: the switch must be INSIDE the try block, but the goto target
    //  for symmetric transfer must be OUTSIDE it)
    try {
        switch (state->__suspend_point) {
        case 0: goto suspend_point_0;   // initial suspend
        case 1: goto suspend_point_1;   // after co_await f(x)
        default: std::unreachable();
        }

    // ══════════════════════════════════════════════════════════════════════
    // RESUME FROM INITIAL SUSPEND (suspend_point = 0)
    // ══════════════════════════════════════════════════════════════════════
    suspend_point_0:
        {
            // Destroy __tmp1 (the suspend_always from initial_suspend)
            // destructor_guard calls destroy() when scope exits (or on exception)
            destructor_guard tmp1_dtor{state->__tmp1};
            state->__tmp1.get().await_resume();  // returns void for suspend_always
        } // __tmp1 destroyed here

        // ── Now executing user code: int fx = co_await f(x); ────────────
        {
            // Evaluate f(x) — stores the returned task in coroutine frame
            state->__s1.__tmp2.construct_from([&] {
                return f(state->x);
            });
            destructor_guard tmp2_dtor{state->__s1.__tmp2};

            // Call operator co_await() on the task to get the awaiter
            state->__s1.__tmp3.construct_from([&] {
                return static_cast<task&&>(state->__s1.__tmp2.get()).operator co_await();
            });
            destructor_guard tmp3_dtor{state->__s1.__tmp3};

            if (!state->__s1.__tmp3.get().await_ready()) {
                // ── SUSPEND POINT 1 ──────────────────────────────────────
                state->__suspend_point = 1;  // record where we are

                // await_suspend returns a coroutine_handle (symmetric transfer)
                auto h = state->__s1.__tmp3.get().await_suspend(
                    std::coroutine_handle<__g_promise_t>::from_promise(state->__promise)
                );

                // CANCEL the destructor guards — coroutine is suspended,
                // __tmp2 and __tmp3 must stay alive in the frame!
                tmp3_dtor.cancel();
                tmp2_dtor.cancel();

                // Return the next coroutine to resume (symmetric transfer)
                // The trampoline in coroutine_handle::resume() will resume it.
                return static_cast<__coroutine_state*>(h.address());
            }

            // If await_ready() returned true (completed synchronously),
            // fall through to suspend_point_1 without actually suspending.
            // Cancel guards here — we'll re-create them after the label.
            tmp3_dtor.cancel();
            tmp2_dtor.cancel();
        }

    // ══════════════════════════════════════════════════════════════════════
    // RESUME AFTER co_await f(x) (suspend_point = 1)
    // ══════════════════════════════════════════════════════════════════════
    suspend_point_1:
        // Re-create destructor guards for __tmp2 and __tmp3
        // (they are still alive in the frame — we cancelled the guards above)
        int fx = [&]() -> decltype(auto) {
            destructor_guard tmp2_dtor{state->__s1.__tmp2};
            destructor_guard tmp3_dtor{state->__s1.__tmp3};

            // await_resume() returns the int result from the task
            // Both guards fire here, destroying __tmp3 then __tmp2
            return state->__s1.__tmp3.get().await_resume();
        }();
        // __tmp3 (awaiter) and __tmp2 (task) are now destroyed

        // ── co_return fx * fx; ──────────────────────────────────────────
        // Calls promise.return_value(fx * fx) then jumps to final suspend
        state->__promise.return_value(fx * fx);
        goto final_suspend;

    } catch (...) {
        // Any exception from the coroutine body lands here
        state->__promise.unhandled_exception();
        goto final_suspend;
    }

// ══════════════════════════════════════════════════════════════════════════
// FINAL SUSPEND (outside try/catch — must be noexcept)
// ══════════════════════════════════════════════════════════════════════════
final_suspend:
    {
        state->__tmp4.construct_from([&]() noexcept {
            return state->__promise.final_suspend();
        });
        destructor_guard tmp4_dtor{state->__tmp4};

        if (!state->__tmp4.get().await_ready()) {
            // Mark as final suspend — resume() is now UB, only destroy() is valid
            state->__suspend_point = 2;
            state->__resume = nullptr;  // done() returns true when __resume == null

            auto h = state->__tmp4.get().await_suspend(
                std::coroutine_handle<__g_promise_t>::from_promise(state->__promise)
            );

            tmp4_dtor.cancel();

            // Symmetric transfer: resume whoever was waiting for us
            // (typically the coroutine that did co_await on this task)
            return static_cast<__coroutine_state*>(h.address());
        }

        state->__tmp4.get().await_resume();
    }

    // Coroutine ran to completion without suspending at final_suspend
    // (unusual — frame must be destroyed manually)
    delete state;

    // Tell the trampoline "we're done, stop looping"
    return static_cast<__coroutine_state*>(std::noop_coroutine().address());
}
Part 5: The Destroy Function
Called when someone destroys a suspended coroutine without resuming it.

cpp
void __g_destroy(__coroutine_state* s) {
    auto* state = static_cast<__g_state*>(s);

    // Jump to the right cleanup based on where we were suspended
    switch (state->__suspend_point) {
    case 0: goto suspend_point_0;  // suspended at initial_suspend
    case 1: goto suspend_point_1;  // suspended at co_await f(x)
    case 2: goto suspend_point_2;  // suspended at final_suspend
    default: std::unreachable();
    }

suspend_point_0:
    // Only __tmp1 (suspend_always from initial_suspend) is alive
    state->__tmp1.destroy();
    goto destroy_state;

suspend_point_1:
    // __tmp2 (task) and __tmp3 (awaiter) are alive
    // Destroy in reverse order of construction
    state->__s1.__tmp3.destroy();
    state->__s1.__tmp2.destroy();
    goto destroy_state;

suspend_point_2:
    // __tmp4 (final_awaiter) is alive
    state->__tmp4.destroy();
    goto destroy_state;

destroy_state:
    // Destroy the entire coroutine frame (promise + parameters + frame)
    delete state;
}
The Trampoline Loop — How Symmetric Transfer Works
This is the magic that prevents stack overflow:

cpp
void coroutine_handle<void>::resume() const {
    __coroutine_state* s = state_;
    do {
        s = s->__resume(s);     // resume returns the NEXT coroutine to run
    } while (s != &__coroutine_state::__noop_coroutine);
    // Stop when we hit the noop coroutine (no more work to do)
}
Execution flow for co_await g(x) where g awaits f:

caller calls handle.resume()
    │
    ▼
trampoline loop starts
    │
    ├─ calls __g_resume(g_state)       [g runs until it hits co_await f(x)]
    │    └─ returns __f_state*         [symmetric transfer to f]
    │
    ├─ calls __f_resume(f_state)       [f runs until it completes]
    │    └─ returns __g_state*         [symmetric transfer back to g via continuation]
    │
    ├─ calls __g_resume(g_state)       [g resumes, runs to final_suspend]
    │    └─ returns noop_coroutine     [done!]
    │
    └─ loop exits, resume() returns
Without the trampoline, each resume() call would add a stack frame. With it, the stack stays constant depth regardless of how many coroutines chain together.

Complete Summary: The Full Pipeline
task g(int x) {                          User writes this
    int fx = co_await f(x);
    co_return fx * fx;
}
                │
                │  compiler transforms into:
                ▼
┌─────────────────────────────────────────────────────┐
│                                                     │
│  __g_state (heap)                                   │
│  ├── __resume  ──────────────────► __g_resume()     │
│  ├── __destroy ──────────────────► __g_destroy()    │
│  ├── __promise                                      │
│  ├── __suspend_point  (0, 1, or 2)                  │
│  ├── x  (parameter copy)                            │
│  └── union { __tmp1, __s1{__tmp2,__tmp3}, __tmp4 }  │
│                                                     │
│  task g(int x)  ← ramp function                     │
│  ├── allocates __g_state                            │
│  ├── calls get_return_object()                      │
│  ├── handles initial_suspend                        │
│  └── returns task to caller                         │
│                                                     │
│  __g_resume()  ← state machine                      │
│  ├── switch(__suspend_point) → goto                 │
│  ├── case 0: resume from initial_suspend            │
│  ├── case 1: resume from co_await f(x)              │
│  └── returns next __coroutine_state* (trampoline)   │
│                                                     │
│  __g_destroy()  ← cleanup at any suspend point      │
│  ├── switch(__suspend_point) → goto                 │
│  ├── destroys exactly the right temporaries         │
│  └── deletes the frame                              │
│                                                     │
└─────────────────────────────────────────────────────┘
Every co_await, co_return, and co_yield you write gets translated by this exact machinery — the compiler is just generating this boilerplate automatically, saving you from a mountain of error-prone state-machine code.