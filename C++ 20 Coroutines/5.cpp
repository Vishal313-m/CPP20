/* co_await*/



// ============================================================
//   C++20 — BUILDING & USING YOUR FIRST COROUTINE TYPE
//   (co_await focus — complete working example with notes)
// ============================================================
//
// COMPILER SETUP (important!):
//
//   GCC 10/11  →  add flag:  -fcoroutines
//   MSVC 2019+ (v16.8.3+)  →  no extra flag needed
//   Online compiler  →  https://wandbox.org  (GCC, works fine)
//
// ============================================================
// ARCHITECTURE REMINDER
// ============================================================
//
//   +---------------------------+
//   |      C++ User Code        |  ← main(), calls do_work()
//   +---------------------------+
//   |      CoroType             |  ← Coroutine type YOU build
//   +---------------------------+
//   |  C++20 Infrastructure     |  ← promise_type, handle, awaitables
//   +---------------------------+
//
// ============================================================
// WHAT HAPPENS STEP BY STEP WHEN YOU CALL A COROUTINE
// ============================================================
//
//  1. Call do_work()
//       └─► Compiler creates promise object on the HEAP
//       └─► Calls promise.get_return_object() → gives you CoroType
//       └─► Calls promise.initial_suspend()
//             → returns suspend_always → PAUSES immediately
//             → returns suspend_never  → RUNS immediately
//
//  2. You call task.m_handle()  OR  task.m_handle.resume()
//       └─► Coroutine resumes from last pause point
//       └─► Runs until next co_await / co_yield / co_return / end
//
//  3. co_await suspend_always{} hit
//       └─► Compiler checks await_ready() → returns false → PAUSE
//       └─► Calls await_suspend(handle) → stores state on HEAP
//       └─► Control returns to caller (main)
//
//  4. task.m_handle.done()
//       └─► false = coroutine still has work to do
//       └─► true  = coroutine finished (DON'T resume!)
//
//  5. ~CoroType() called (destructor)
//       └─► m_handle.destroy() → frees heap memory
//
// ============================================================
// TWO WAYS TO RESUME A COROUTINE HANDLE
// ============================================================
//
//   task.m_handle();          // call handle like a function
//   task.m_handle.resume();   // call .resume() explicitly
//
//   Both do EXACTLY the same thing.
//
// ============================================================
// AWAITABLES — what you pass to co_await
// ============================================================
//
//   struct suspend_never {                    // DON'T suspend
//       constexpr bool await_ready()  { return true;  }  ← true = skip suspend
//       constexpr void await_suspend(coroutine_handle<>) {}
//       constexpr void await_resume() {}
//   };
//
//   struct suspend_always {                   // ALWAYS suspend
//       constexpr bool await_ready()  { return false; }  ← false = do suspend
//       constexpr void await_suspend(coroutine_handle<>) {}
//       constexpr void await_resume() {}
//   };
//
//   await_ready()   → bool: should we SKIP suspension? (true=skip, false=suspend)
//   await_suspend() → called when suspending; can grab the handle
//   await_resume()  → called when resuming; can return a value
//
// ============================================================
// FULL WORKING CODE
// ============================================================

#include <iostream>
#include <coroutine>

// ------------------------------------------------------------
// PART 1: The Coroutine TYPE (the machinery you must build)
// ------------------------------------------------------------
struct CoroType {

    // --------------------------------------------------------
    // Nested promise_type — MUST be named exactly this
    // Controls behavior at key coroutine lifecycle points
    // --------------------------------------------------------
    struct promise_type {

        // Called first: returns the CoroType object to caller
        CoroType get_return_object() {
            return CoroType(this);   // passes promise pointer to constructor
        }

        // Called before coroutine body runs.
        // suspend_always → pause on first call (most common)
        // suspend_never  → run immediately on first call
        std::suspend_always initial_suspend() { return {}; }

        // Called when coroutine reaches the end.
        // suspend_always → pause at end (you destroy manually)
        // suspend_never  → auto-destroy state at end
        std::suspend_always final_suspend() noexcept { return {}; }

        // Any exception thrown inside the coroutine comes here.
        // rethrow_exception → sends it to whoever called the coroutine
        void unhandled_exception() noexcept {
            std::rethrow_exception(std::current_exception());
        }

        // Needed because our coroutine ends without co_return value
        // Enables: falling off end of coroutine body (implicit co_return)
        void return_void() {}
    };

    // --------------------------------------------------------
    // Constructor: builds the handle from the promise pointer
    // --------------------------------------------------------
    CoroType(promise_type* p)
        : m_handle(std::coroutine_handle<promise_type>::from_promise(*p)) {}

    // --------------------------------------------------------
    // Destructor: MUST destroy the coroutine frame on heap!
    // If you skip this → memory leak
    // --------------------------------------------------------
    ~CoroType() {
        std::cout << "Handle destroyed..." << std::endl;
        m_handle.destroy();
    }

    // --------------------------------------------------------
    // The handle — your remote control for the coroutine
    // --------------------------------------------------------
    std::coroutine_handle<promise_type> m_handle;
};

// ------------------------------------------------------------
// PART 2: The coroutine FUNCTION
// Return type = CoroType → tells compiler this is a coroutine
// co_await inside → also confirms it's a coroutine
// ------------------------------------------------------------
CoroType do_work() {
    std::cout << "Doing first thing..." << std::endl;
    co_await std::suspend_always{};   // ← suspension point #1

    std::cout << "Doing second thing..." << std::endl;
    co_await std::suspend_always{};   // ← suspension point #2

    std::cout << "Doing Third thing..." << std::endl;
    // coroutine ends here (return_void called implicitly)
}

// ------------------------------------------------------------
// PART 3: User code — main() controls the coroutine
// ------------------------------------------------------------
int main() {

    // Creates coroutine, but it does NOT start yet
    // because initial_suspend() = suspend_always
    auto task = do_work();

    std::cout << std::boolalpha;   // print true/false not 1/0

    // ── Resume #1 ──────────────────────────────────────────
    // Syntax: call handle like a function
    task.m_handle();
    // Runs: "Doing first thing..."
    // Hits: co_await suspend_always → PAUSES
    std::cout << "coro done: " << task.m_handle.done() << std::endl;
    // Output: false  (still has work to do)

    // ── Resume #2 ──────────────────────────────────────────
    // Syntax: call .resume() explicitly (same result)
    task.m_handle.resume();
    // Runs: "Doing second thing..."
    // Hits: co_await suspend_always → PAUSES
    std::cout << "coro done: " << task.m_handle.done() << std::endl;
    // Output: false

    // ── Resume #3 ──────────────────────────────────────────
    task.m_handle.resume();
    // Runs: "Doing Third thing..."
    // Reaches END of function → coroutine DONE
    std::cout << "coro done: " << task.m_handle.done() << std::endl;
    // Output: true

    // ⚠ DO NOT DO THIS:
    // task.m_handle.resume();  // UNDEFINED BEHAVIOR — coroutine is done!
    //                          // May crash, may print garbage, may corrupt memory

    std::cout << "Done!" << std::endl;
    return 0;
    // ~CoroType() called here → m_handle.destroy() → heap freed
}

// ============================================================
// COMPLETE EXECUTION FLOW DIAGRAM
// ============================================================
//
//  main()                         do_work() coroutine
//  ──────────────────────         ──────────────────────────
//
//  auto task = do_work()
//    │ promise created on heap
//    │ get_return_object() called
//    │ initial_suspend() → suspend_always
//    └──────────────────────────► [PAUSED — before body starts]
//
//  task.m_handle()  ────────────► resume from start
//                                 "Doing first thing..."
//                                 co_await suspend_always
//                                   await_ready() → false → PAUSE
//                                 [PAUSED at point #1]
//  coro done: false
//
//  task.m_handle.resume() ──────► resume from point #1
//                                 "Doing second thing..."
//                                 co_await suspend_always
//                                   await_ready() → false → PAUSE
//                                 [PAUSED at point #2]
//  coro done: false
//
//  task.m_handle.resume() ──────► resume from point #2
//                                 "Doing Third thing..."
//                                 [END OF COROUTINE]
//                                 final_suspend() called
//                                 [DONE]
//  coro done: true
//
//  "Done!"
//  ~CoroType() → m_handle.destroy() → heap freed
//  "Handle destroyed..."
//
// ============================================================
// EXPECTED CONSOLE OUTPUT
// ============================================================
//
//   Doing first thing...
//   coro done: false
//   Doing second thing...
//   coro done: false
//   Doing Third thing...
//   coro done: true
//   Done!
//   Handle destroyed...
//
// ============================================================
// initial_suspend BEHAVIOR — suspend_always vs suspend_never
// ============================================================
//
//   initial_suspend returns suspend_always (default/recommended):
//     task = do_work()  → PAUSE before body
//     task.m_handle()   → runs "Doing first thing..."
//
//   initial_suspend returns suspend_never:
//     task = do_work()  → IMMEDIATELY runs "Doing first thing..."
//                          then pauses at first co_await
//     task.m_handle()   → runs "Doing second thing..."
//
//   Most coroutine types use suspend_always as the default
//   so the caller decides WHEN to start the coroutine.
//
// ============================================================
// MINIMUM REQUIRED promise_type METHODS (or code won't compile)
// ============================================================
//
//   get_return_object()    ← without this: compiler error
//   initial_suspend()      ← without this: compiler error
//   final_suspend()        ← without this: compiler error
//   unhandled_exception()  ← without this: compiler error
//   return_void() OR return_value()  ← depends on co_return usage
//
// ============================================================
// SAFE RESUME PATTERN (always check before resuming)
// ============================================================
//
//   if (!task.m_handle.done()) {
//       task.m_handle.resume();
//   }
//
// ============================================================