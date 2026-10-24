/* co_return */


// ============================================================
//   C++20 — co_return OPERATOR (Complete Notes)
// ============================================================
//
// WHAT co_return DOES:
//   Ends (completes) the coroutine.
//   Optionally stores a final value in promise.m_value.
//   After this, done() == true. NEVER resume again!
//
// ============================================================
// TWO FORMS OF co_return
// ============================================================
//
//   co_return val;   → calls promise.return_value(val)
//                      stores val, then ENDS coroutine
//
//   co_return;       → calls promise.return_void()
//                      no value stored, ENDS coroutine
//
// ============================================================
// WHAT YOU NEED TO ADD TO promise_type FOR co_return
// ============================================================
//
//   Form 1 — co_return val;   → add return_value():
//     void return_value(int val) {
//         m_value = val;   // store the final return value
//     }
//
//   Form 2 — co_return;       → add return_void():
//     void return_void() {
//         // nothing to store, just ends the coroutine
//     }
//
// ⚠ COMPILER QUIRK:
//   GCC   → accepts BOTH return_value() AND return_void() together
//   MSVC  → ERROR if both are present simultaneously
//
//   CROSS-COMPILER SAFE RULE:
//     Pick ONE: either return_value() OR return_void(), not both.
//     If you need both, use return_value() and ignore the value
//     when co_return; (no operand) is used.
//
// ============================================================
// DIFFERENCE: co_yield vs co_return
// ============================================================
//
//  +----------------+--------------------------+------------+
//  |                |  co_yield val            | co_return  |
//  +----------------+--------------------------+------------+
//  | Promise method | yield_value(val)         | return_value(val)
//  |                |                          | OR return_void()
//  | Returns        | suspend_always (pauses)  | void (ends)|
//  | Coroutine ends?| NO — resumes next time   | YES — DONE |
//  | done() after  | false                    | true       |
//  +----------------+--------------------------+------------+
//
// ============================================================
// FULL WORKING CODE (cross-compiler safe version)
// ============================================================

#include <iostream>
#include <coroutine>

struct CoroType {
    struct promise_type {

        int m_value;

        CoroType get_return_object() { return CoroType(this); }
        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }

        void unhandled_exception() noexcept {
            std::rethrow_exception(std::current_exception());
        }

        // Enables: co_yield val;
        std::suspend_always yield_value(int val) {
            m_value = val;
            return {};
        }

        // Enables: co_return;  (no value)
        // ── Cross-compiler safe: use ONLY ONE of these ──────
        void return_void() {
            std::cout << "Returning void..." << std::endl;
        }

        // Enables: co_return val;  (with value)
        // Comment out return_void above if using this with MSVC
        /*
        void return_value(int val) {
            std::cout << "Returning value: " << val << std::endl;
            m_value = val;
        }
        */
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
// Coroutine: yields 3 values then ends with co_return
// ------------------------------------------------------------
CoroType do_work() {
    std::cout << "Starting the coroutine..." << std::endl;
    co_yield 1;    // pause, store 1
    co_yield 2;    // pause, store 2
    co_yield 3;    // pause, store 3
    co_return;     // END coroutine (calls return_void)
    // Note: if the function just falls off the end without
    // co_return, the compiler inserts an implicit co_return;
    // which is why return_void() is required.
}

// ------------------------------------------------------------
// main
// ------------------------------------------------------------
int main() {
    auto task = do_work();   // PAUSED immediately (initial_suspend)
    std::cout << std::boolalpha;

    // ── Resume #1 → hits co_yield 1 ───────────────────────
    task.m_handle();
    std::cout << "value : " << task.m_handle.promise().m_value << std::endl;
    std::cout << std::endl;
    // Output: Starting the coroutine...
    //         value : 1

    // ── Resume #2 → hits co_yield 2 ───────────────────────
    task.m_handle();
    std::cout << "value : " << task.m_handle.promise().m_value << std::endl;
    std::cout << "coro done : " << task.m_handle.done() << std::endl;
    std::cout << std::endl;
    // Output: value : 2
    //         coro done : false

    // ── Resume #3 → hits co_yield 3 ───────────────────────
    task.m_handle();
    std::cout << "value : " << task.m_handle.promise().m_value << std::endl;
    std::cout << "coro done : " << task.m_handle.done() << std::endl;
    std::cout << std::endl;
    // Output: value : 3
    //         coro done : false  ← paused AT yield 3, not done yet

    // ── Resume #4 → hits co_return; → DONE ────────────────
    task.m_handle();
    std::cout << "value : " << task.m_handle.promise().m_value << std::endl;
    std::cout << "coro done : " << task.m_handle.done() << std::endl;
    // Output: Returning void...
    //         value : 3          ← stale value from last co_yield
    //         coro done : true   ← coroutine is DONE

    // ⚠ task.m_handle();  ← NEVER after done() == true → CRASH / UB

    std::cout << "Done!" << std::endl;
    return 0;
    // ~CoroType() → m_handle.destroy() → "Handle destroyed..."
}

// ============================================================
// COMPLETE EXECUTION FLOW DIAGRAM
// ============================================================
//
//  main()                        do_work() coroutine
//  ──────────────────            ─────────────────────────────
//
//  auto task = do_work()
//    initial_suspend() ────────► [PAUSED — before body]
//
//  task.m_handle() ───────────► resume
//                               "Starting the coroutine..."
//                               co_yield 1
//                                 yield_value(1) → m_value=1
//                               [PAUSED]
//  value : 1
//
//  task.m_handle() ───────────► resume
//                               co_yield 2
//                                 yield_value(2) → m_value=2
//                               [PAUSED]
//  value : 2  |  done: false
//
//  task.m_handle() ───────────► resume
//                               co_yield 3
//                                 yield_value(3) → m_value=3
//                               [PAUSED]
//  value : 3  |  done: false
//
//  task.m_handle() ───────────► resume
//                               co_return;
//                                 return_void() called
//                               "Returning void..."
//                               final_suspend() → [DONE]
//  value : 3 (stale)  |  done: true
//
//  "Done!"
//  ~CoroType() → m_handle.destroy() → "Handle destroyed..."
//
// ============================================================
// EXPECTED CONSOLE OUTPUT
// ============================================================
//
//   Starting the coroutine...
//   value : 1
//
//   value : 2
//   coro done : false
//
//   value : 3
//   coro done : false
//
//   Returning void...
//   value : 3
//   coro done : true
//   Done!
//   Handle destroyed...
//
// ============================================================
// WHY return_void() IS REQUIRED EVEN WITHOUT EXPLICIT co_return
// ============================================================
//
//   When the coroutine body ends without co_return,
//   the compiler INSERTS an implicit:
//       co_return;
//   This calls return_void() automatically.
//   If return_void() is missing → compiler error (especially MSVC).
//
//   So even if you write:
//     CoroType do_work() {
//         co_yield 1;
//         // no co_return here
//     }
//   The compiler treats the end as: co_return; → needs return_void()
//
// ============================================================
// SUMMARY: ALL THREE LECTURES COMBINED
// ============================================================
//
//   Operator      → promise_type method needed   → Effect
//   ──────────────────────────────────────────────────────
//   co_await x    → (awaitable handles it)       → pause, wait
//   co_yield val  → yield_value(val)             → pause + store val
//   co_return val → return_value(val)            → END + store val
//   co_return;    → return_void()                → END, no value
//
//   Reading back stored value:
//     task.m_handle.promise().m_value
//
// ============================================================
// MSVC vs GCC COMPATIBILITY TABLE
// ============================================================
//
//   Situation                        | GCC  | MSVC
//   ---------------------------------|------|------
//   Only return_void()               |  ✅  |  ✅
//   Only return_value(val)           |  ✅  |  ✅
//   Both return_void() + return_value|  ✅  |  ❌ ERROR
//
//   Recommendation: choose ONE based on your use case.
//   If you need co_return; (void), use return_void().
//   If you need co_return val;, use return_value().
//
// ============================================================