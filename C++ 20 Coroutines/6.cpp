/* co_yield*/


// ============================================================
//   C++20 — co_yield OPERATOR (Complete Notes)
// ============================================================
//
// WHAT co_yield DOES:
//   Pauses the coroutine AND stores a value in the promise_type.
//   Caller accesses that value via: handle.promise().m_value
//
// KEY INSIGHT — co_yield is just syntactic sugar:
//
//   co_yield 2;
//        │
//        └──► compiler transforms to ──►  co_await promise.yield_value(2);
//
//   So co_yield calls yield_value() on the promise,
//   then co_awaits whatever yield_value() returns.
//   If yield_value() returns suspend_always → coroutine PAUSES.
//   If yield_value() returns suspend_never  → coroutine CONTINUES.
//
// ============================================================
// WHAT YOU NEED TO ADD TO CoroType FOR co_yield TO WORK
// ============================================================
//
//   Inside promise_type, add:
//
//   1. A member variable to STORE the yielded value:
//        int m_value;
//
//   2. The yield_value() method:
//        std::suspend_always yield_value(int val) {
//            m_value = val;   // store the yielded value
//            return {};       // suspend_always → pause coroutine
//        }
//
//   That's it. Everything else stays the same as before.
//
// ============================================================
// HOW TO ACCESS THE YIELDED VALUE FROM main()
// ============================================================
//
//   task.m_handle.promise().m_value
//   │            │          │
//   │            │          └── the stored int value
//   │            └──── gets the promise_type object
//   └─── your CoroType object
//
// ============================================================
// FULL WORKING CODE
// ============================================================

#include <iostream>
#include <coroutine>

struct CoroType {
    struct promise_type {

        int m_value;   // ← NEW: stores the co_yield value

        CoroType get_return_object() {
            return CoroType(this);
        }

        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }

        void unhandled_exception() noexcept {
            std::rethrow_exception(std::current_exception());
        }

        // ── NEW: required for co_yield to work ──────────────
        // Called by compiler when co_yield val; is hit
        // Stores the value, then suspends
        std::suspend_always yield_value(int val) {
            m_value = val;   // save yielded value into promise
            return {};       // suspend_always → pause coroutine
        }
        // ─────────────────────────────────────────────────────

        // NOTE: return_void() removed here intentionally for
        // this example (GCC accepts it; MSVC requires it).
        // Add it back for cross-compiler support:
        // void return_void() {}
    };

    CoroType(promise_type* p)
        : m_handle(std::coroutine_handle<promise_type>::from_promise(*p)) {}

    ~CoroType() {
        std::cout << "Handle destroyed..." << std::endl;
        m_handle.destroy();
    }

    std::coroutine_handle<promise_type> m_handle;
};

// ------------------------------------------------------------
// Coroutine function using co_yield
// ------------------------------------------------------------
CoroType do_work() {
    co_yield 1;   // stores 1 in promise.m_value, then PAUSES
    co_yield 2;   // stores 2 in promise.m_value, then PAUSES
    co_yield 3;   // stores 3 in promise.m_value, then PAUSES
    // coroutine ends here (implicit co_return)
}

// ------------------------------------------------------------
// main — resume and read yielded values
// ------------------------------------------------------------
int main() {
    auto task = do_work();   // coroutine created, PAUSED immediately
                             // (initial_suspend = suspend_always)
    std::cout << std::boolalpha;

    // ── Resume #1 ──────────────────────────────────────────
    task.m_handle();         // resume → runs → hits co_yield 1 → PAUSE
    std::cout << "value : " << task.m_handle.promise().m_value << std::endl;
    // Output: value : 1

    // ── Resume #2 ──────────────────────────────────────────
    task.m_handle();         // resume → runs → hits co_yield 2 → PAUSE
    std::cout << "value : " << task.m_handle.promise().m_value << std::endl;
    // Output: value : 2

    // ── Resume #3 ──────────────────────────────────────────
    task.m_handle();         // resume → runs → hits co_yield 3 → PAUSE
    std::cout << "value : " << task.m_handle.promise().m_value << std::endl;
    std::cout << "coro done: " << task.m_handle.done() << std::endl;
    // Output: value : 3
    // Output: coro done: false  ← still paused AT co_yield 3, not finished yet

    // ── Resume #4 ──────────────────────────────────────────
    task.m_handle.resume();  // resume → runs → hits END of function → DONE
    std::cout << "coro done: " << task.m_handle.done() << std::endl;
    // Output: coro done: true

    // ⚠ task.m_handle.resume();  ← NEVER do this after done() == true
    //                               UNDEFINED BEHAVIOR / CRASH

    std::cout << "Done!" << std::endl;
    return 0;
    // ~CoroType() → m_handle.destroy() → "Handle destroyed..."
}

// ============================================================
// EXECUTION FLOW DIAGRAM
// ============================================================
//
//  main()                         do_work() coroutine
//  ──────────────────             ─────────────────────────────
//
//  auto task = do_work()
//    initial_suspend() ─────────► [PAUSED — before body]
//
//  task.m_handle() ────────────► resume
//                                co_yield 1
//                                  └─► yield_value(1) called
//                                  └─► m_value = 1
//                                  └─► returns suspend_always
//                                [PAUSED]
//  promise().m_value → 1
//
//  task.m_handle() ────────────► resume
//                                co_yield 2
//                                  └─► yield_value(2) called
//                                  └─► m_value = 2
//                                [PAUSED]
//  promise().m_value → 2
//
//  task.m_handle() ────────────► resume
//                                co_yield 3
//                                  └─► yield_value(3) called
//                                  └─► m_value = 3
//                                [PAUSED]
//  promise().m_value → 3
//  done() → false  ← paused, not finished
//
//  task.m_handle.resume() ─────► resume
//                                [END OF FUNCTION]
//                                final_suspend() called
//                                [DONE]
//  done() → true
//
//  "Done!"
//  ~CoroType() → m_handle.destroy()
//  "Handle destroyed..."
//
// ============================================================
// EXPECTED CONSOLE OUTPUT
// ============================================================
//
//   value : 1
//   value : 2
//   value : 3
//   coro done: false
//   coro done: true
//   Done!
//   Handle destroyed...
//
// ============================================================
// IMPORTANT: done() == false AFTER co_yield 3 (not after resume)
// ============================================================
//
//   After task.m_handle() hits co_yield 3:
//     → coroutine is PAUSED (suspended), NOT finished
//     → done() returns false
//
//   After the 4th resume (no more co_yield, hits end):
//     → coroutine FINISHED
//     → done() returns true
//
//   So you need ONE MORE resume after the last co_yield
//   to actually finish the coroutine.
//
// ============================================================
// SUMMARY: WHAT CHANGED FROM PREVIOUS LECTURE (co_await)
// ============================================================
//
//   Previous (co_await lecture):         This lecture (co_yield):
//   ──────────────────────────           ──────────────────────
//   No m_value member                   Added: int m_value;
//   No yield_value() method             Added: yield_value(int val)
//   Used: co_await suspend_always{}     Used: co_yield 1; co_yield 2; ...
//   Value not accessible to caller      Value via: handle.promise().m_value
//
// ============================================================
// CROSS-COMPILER NOTE
// ============================================================
//
//   GCC   → compiles without return_void() if co_return is absent
//   MSVC  → REQUIRES return_void() even if not explicitly used
//
//   SAFE: always add this to promise_type:
//     void return_void() {}
//
// ============================================================