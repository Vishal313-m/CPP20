/*  Coroutine INFRASTRUCTURE*/





// ============================================================
//         C++20 COROUTINES - COMPLETE NOTES
// ============================================================
//
// ARCHITECTURE OVERVIEW:
//
//   +---------------------------+
//   |      C++ User Code        |  <-- You write this (uses coroutine types)
//   +---------------------------+
//   |     Coroutine Types       |  <-- e.g. generator<T>, CoroType
//   +---------------------------+
//   |  Coroutine Infrastructure |  <-- C++20 provides this (low-level)
//   +---------------------------+
//
// C++20 gives you the INFRASTRUCTURE (rules/machinery).
// You (or a library) build COROUTINE TYPES on top of it.
// Then USER CODE consumes those types.
//
// ============================================================
// WHAT IS A COROUTINE?
// ============================================================
//
// A coroutine is a function that can:
//   - PAUSE  (suspend) its execution mid-way
//   - RESUME from where it paused
//   - RETURN intermediate values while paused
//
// When paused, its state (local vars, parameters, resume point)
// is saved on the HEAP.
//
// Three coroutine keywords:
//   co_yield  → yield a value and pause
//   co_return → return final value and finish
//   co_await  → wait on something (may pause)
//
// ============================================================
// THREE PILLARS OF A COROUTINE
// ============================================================
//
//   +----------------+   +------------------+   +----------+
//   |  Promise Type  |   | Coroutine Handle |   | Awaiter  |
//   +----------------+   +------------------+   +----------+
//   Controls behavior    Used to resume/         Operand to
//   (suspend? return?)   destroy coroutine       co_await
//
// ============================================================
// PILLAR 1: PROMISE TYPE
// ============================================================
//
// - A nested struct/class named EXACTLY "promise_type" inside your coroutine type.
// - Controls HOW your coroutine behaves at key moments.
//
// Required methods:
// -----------------------------------------------------------------
// | Method                  | Purpose                            |
// |-------------------------|------------------------------------|
// | get_return_object()     | Returns the coroutine type object  |
// | initial_suspend()       | Suspend on first call? (yes/no)    |
// | final_suspend()         | Destroy state at end? (yes/no)     |
// | unhandled_exception()   | Handle exceptions from coroutine   |
// | return_void()           | Enables: co_return;                |
// | return_value(v)         | Enables: co_return v;              |
// | yield_value(v)          | Enables: co_yield v;               |
// -----------------------------------------------------------------
//
// initial_suspend() returns:
//   std::suspend_always{} → coroutine pauses immediately on first call
//   std::suspend_never{}  → coroutine starts running immediately
//
// ============================================================
// PILLAR 2: COROUTINE HANDLE
// ============================================================
//
// std::coroutine_handle<promise_type>
//
// - Holds a reference to the coroutine state on the heap.
// - Used to RESUME or DESTROY the coroutine.
//
// Key operations:
//   handle.resume()   → continue execution from pause point
//   handle()          → same as resume()
//   handle.destroy()  → free heap memory (MUST call in destructor!)
//   handle.done()     → true if coroutine has finished
//
// Created from a promise:
//   std::coroutine_handle<promise_type>::from_promise(*p)
//
// ============================================================
// PILLAR 3: AWAITER (Awaitables)
// ============================================================
//
// An awaitable is the operand to co_await.
// Must implement three methods:
// -----------------------------------------------------------------
// | Method           | Purpose                                   |
// |------------------|-------------------------------------------|
// | await_ready()    | Returns bool. If true → don't suspend.    |
// |                  | If false → call await_suspend()           |
// | await_suspend()  | Actually suspends the coroutine (or       |
// |                  | schedules it for destruction)             |
// | await_resume()   | Returns the result of the co_await expr   |
// -----------------------------------------------------------------
//
// Built-in awaitables in C++20:
//
//   std::suspend_never  → await_ready() returns true  → NEVER suspends
//   std::suspend_always → await_ready() returns false → ALWAYS suspends
//
// Their definitions:
//
//   struct suspend_never {
//       constexpr bool await_ready()  const noexcept { return true; }
//       constexpr void await_suspend(coroutine_handle<>) const noexcept {}
//       constexpr void await_resume() const noexcept {}
//   };
//
//   struct suspend_always {
//       constexpr bool await_ready()  const noexcept { return false; }
//       constexpr void await_suspend(coroutine_handle<>) const noexcept {}
//       constexpr void await_resume() const noexcept {}
//   };
//
// Custom awaitable (overload co_await operator):
//
//   struct My_Awaitable {
//       auto operator co_await() { return std::suspend_always{}; }
//   };
//
// ============================================================
// COMPLETE COROUTINE TYPE EXAMPLE (with co_await)
// ============================================================

#include <coroutine>
#include <iostream>

// -----------------------------------------------------------
// Step 1: Define the Coroutine Type
// -----------------------------------------------------------
struct CoroType {

    // -------------------------------------------------------
    // Step 2: Nested promise_type (MUST be named exactly this)
    // -------------------------------------------------------
    struct promise_type {

        // Returns the coroutine object to the caller
        CoroType get_return_object() {
            return CoroType(this);  // passes promise* to constructor
        }

        // Suspend immediately on first call (most common)
        std::suspend_always initial_suspend() { return {}; }

        // Don't auto-destroy state at end (we destroy manually)
        std::suspend_always final_suspend() noexcept { return {}; }

        // Re-throw any exception thrown inside the coroutine
        void unhandled_exception() noexcept {
            std::rethrow_exception(std::current_exception());
        }

        // Enables: co_return;  (no value)
        void return_void() {}
    };

    // -------------------------------------------------------
    // Step 3: Constructor — build handle from promise pointer
    // -------------------------------------------------------
    CoroType(promise_type* p)
        : m_handle(std::coroutine_handle<promise_type>::from_promise(*p)) {}

    // -------------------------------------------------------
    // Step 4: Destructor — MUST destroy coroutine state!
    // -------------------------------------------------------
    ~CoroType() {
        std::cout << "Handle destroyed..." << std::endl;
        m_handle.destroy();  // frees heap memory
    }

    // -------------------------------------------------------
    // Step 5: The coroutine handle member variable
    // -------------------------------------------------------
    std::coroutine_handle<promise_type> m_handle;
};

// -----------------------------------------------------------
// Step 6: A coroutine function — returns CoroType
// -----------------------------------------------------------
CoroType do_work() {
    std::cout << "Doing first thing..." << std::endl;
    co_await std::suspend_always{};  // pause here

    std::cout << "Doing second thing..." << std::endl;
    co_await std::suspend_always{};  // pause here

    std::cout << "Doing third thing..." << std::endl;
    // coroutine ends here
}

// -----------------------------------------------------------
// Step 7: User code — call and resume manually
// -----------------------------------------------------------
int main() {
    CoroType coro = do_work();  // coroutine created but NOT started yet
                                // (because initial_suspend = suspend_always)

    std::cout << "Resuming 1st time..." << std::endl;
    coro.m_handle.resume();     // runs → prints "Doing first thing..."
                                // → hits co_await → pauses

    std::cout << "Resuming 2nd time..." << std::endl;
    coro.m_handle.resume();     // runs → prints "Doing second thing..."
                                // → hits co_await → pauses

    std::cout << "Resuming 3rd time..." << std::endl;
    coro.m_handle.resume();     // runs → prints "Doing third thing..."
                                // → reaches end → coroutine done

    // ~CoroType() destructor called → m_handle.destroy()
    return 0;
}

// ============================================================
// EXECUTION FLOW DIAGRAM (do_work example)
// ============================================================
//
//  main()                    do_work() coroutine
//  ──────                    ──────────────────
//  CoroType coro = do_work() ──► [CREATED, state on HEAP]
//                                 initial_suspend → PAUSE ◄──┐
//                                                             │
//  coro.m_handle.resume() ──────────────────────────────────►│
//                                 "Doing first thing..."      │
//                                 co_await suspend_always     │
//                                 PAUSE ◄────────────────────┐│
//                                                            ││
//  coro.m_handle.resume() ───────────────────────────────────┘│
//                                 "Doing second thing..."      │
//                                 co_await suspend_always      │
//                                 PAUSE ◄──────────────────────┘
//                                                             
//  coro.m_handle.resume() ──► "Doing third thing..."
//                             [DONE] coroutine finishes
//
//  ~CoroType() → m_handle.destroy() → heap memory freed
//
// ============================================================
// co_yield EXAMPLE (generator pattern)
// ============================================================
//
// A generator yields values one at a time.
// Needs yield_value() in promise_type.
//
//   generator<int> generate_numbers() {
//       co_yield 10;   // return 10, pause
//       std::cout << "After stop point #1" << std::endl;
//
//       co_yield 20;   // return 20, pause
//       std::cout << "After stop point #2" << std::endl;
//
//       co_yield 30;   // return 30, pause
//       std::cout << "After stop point #3" << std::endl;
//   }
//
// Each co_yield:
//   1. Stores value somewhere accessible
//   2. Pauses the coroutine
//   3. Returns control to caller
//   4. On next resume, continues AFTER the co_yield line
//
// ============================================================
// KEY RULES / GOTCHAS
// ============================================================
//
// 1. ALWAYS call m_handle.destroy() in destructor → prevent memory leak
// 2. NEVER resume a coroutine that is done → undefined behavior
// 3. promise_type MUST be named exactly "promise_type"
// 4. get_return_object() MUST return your coroutine type
// 5. initial_suspend / final_suspend MUST return an awaitable
// 6. C++20 gives infrastructure ONLY — no ready-made generator<T>
//    → Use third-party libs like cppcoro for production use
//
// ============================================================
// SUMMARY TABLE: Which method enables which operator?
// ============================================================
//
//   yield_value(v)   → enables: co_yield v;
//   return_value(v)  → enables: co_return v;
//   return_void()    → enables: co_return;
//   initial_suspend  → controls: pause on first call?
//   final_suspend    → controls: pause/destroy at end?
//
// ============================================================
// THIRD-PARTY LIBRARIES (if you don't want to build your own)
// ============================================================
//
//   cppcoro  → https://github.com/lewissbaker/cppcoro
//              Provides: generator<T>, task<T>, async_generator, etc.
//
//   C++23    → Expected to include high-level coroutine types built-in
//              (std::generator<T> is confirmed in C++23!)
//
// ============================================================