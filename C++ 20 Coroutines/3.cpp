// ============================================================
//      C++20 COROUTINE KEYWORDS - COMPLETE NOTES
// ============================================================
//
// C++20 introduces THREE keywords to pause/resume coroutines:
//
//  +------------+------------------------------------------+
//  |  Keyword   |  What it does                            |
//  +------------+------------------------------------------+
//  | co_yield   | Pause + return a value (can resume)      |
//  | co_return  | End coroutine + optionally return value  |
//  | co_await   | Pause + wait for something (no value)    |
//  +------------+------------------------------------------+
//
// RULE: If a function contains ANY of these keywords,
//       it automatically becomes a coroutine. No other
//       special syntax needed.
//
// ============================================================
// FUNCTIONS THAT CANNOT BE COROUTINES
// ============================================================
//
//   constexpr functions    ← compile-time, can't pause
//   constructors           ← object setup, can't pause
//   destructors            ← object teardown, can't pause
//   main()                 ← entry point, can't pause
//
// ============================================================
// WHAT HAPPENS WHEN A COROUTINE PAUSES?
// ============================================================
//
//  State saved on the HEAP includes:
//
//   +-------------------------+
//   |  Function parameters    |
//   |  Local variables        |
//   |  Resumption point       |  ← where to continue from
//   |  Coroutine handle ref   |  ← used to resume/destroy
//   +-------------------------+
//
//  The COROUTINE HANDLE is your only contact point
//  to resume or destroy that saved state.
//
// ============================================================
// KEYWORD 1: co_yield
// ============================================================
//
// DEFINITION:
//   Suspends the coroutine AND returns a value to the caller.
//   Execution resumes AFTER the co_yield line on next resume.
//
// SYNTAX:
//   co_yield <expression>;
//
// ANALOGY:
//   Like a vending machine — you press a button (resume),
//   it gives you one item (yielded value), then waits again.
//
// ------ EXAMPLE 1: Sequential yields ----------------------

// NOTE: coro[int] is a placeholder for your coroutine type
//       (e.g., generator<int>). C++20 doesn't provide one
//       built-in — you build it or use a library like cppcoro.

// coro[int] func1() {
//     co_yield 45;    // pause, return 45
//     co_yield 46;    // pause, return 46
//     co_yield 47;    // pause, return 47
//     co_return 48;   // END coroutine, return 48
// }
//
// int main() {
//     auto f1 = func1();       // coroutine created, NOT started yet
//
//     std::cout << f1();       // resumes → hits co_yield 45 → prints 45
//     std::cout << f1();       // resumes → hits co_yield 46 → prints 46
//     std::cout << f1();       // resumes → hits co_yield 47 → prints 47
//     std::cout << f1();       // resumes → hits co_return 48 → prints 48
//     std::cout << f1();       // ⚠ UNDEFINED BEHAVIOR! Coroutine is DONE
//
//     return 0;
// }
//
// EXECUTION FLOW (func1):
//
//  Call f1()    Call f1()    Call f1()    Call f1()    Call f1()
//     │            │            │            │            │
//     ▼            ▼            ▼            ▼            ▼
//  co_yield 45  co_yield 46  co_yield 47  co_return 48  ⚠ UB!
//  [PAUSE]      [PAUSE]      [PAUSE]      [DONE]
//  return 45    return 46    return 47    return 48
//
// ------ EXAMPLE 2: Infinite generator with loop ------------

// coro[int] func2() {
//     int start{1};
//     while(true) {
//         co_yield start++;   // yield 1, 2, 3, 4 ... forever
//     }
// }
//
// int main() {
//     auto f2 = func2();      // coroutine handle, not started
//
//     for(auto i{}; i < 10; ++i) {
//         std::cout << f2() << std::endl;   // prints 1 to 10
//     }
//     return 0;
// }
//
// KEY INSIGHT:
//   The infinite loop inside func2 is SAFE because co_yield
//   pauses after each iteration. The loop only advances when
//   YOU call resume. This is "lazy evaluation."
//
// ============================================================
// KEYWORD 2: co_return
// ============================================================
//
// DEFINITION:
//   Ends (completes) the coroutine. Optionally returns a value.
//   After this, the coroutine is DONE — never resume it again!
//
// SYNTAX:
//   co_return <expression>;   // return with value
//   co_return;                // return without value (void)
//
// ------ EXAMPLE: co_return with value ---------------------

// coro[int] func3() {
//     co_return 55;   // ends immediately, returns 55
// }
//
// int main() {
//     auto f3 = func3();
//     std::cout << f3() << std::endl;   // prints 55
//     std::cout << f3() << std::endl;   // ⚠ DISASTER! Already done.
//     return 0;
// }
//
// COMPARISON TABLE:
//
//  +-------------+------------------+--------------------+
//  |             |  return (normal) |  co_return         |
//  +-------------+------------------+--------------------+
//  | Ends func?  |  Yes             |  Yes               |
//  | Can resume? |  No (it's done)  |  No (UB if tried)  |
//  | State heap? |  No              |  Yes (until done)  |
//  | In coro?    |  Not allowed     |  Required          |
//  +-------------+------------------+--------------------+
//
// ============================================================
// KEYWORD 3: co_await
// ============================================================
//
// DEFINITION:
//   Suspends the coroutine and WAITS for something to complete.
//   Does NOT return a value by itself (unlike co_yield).
//   Used when you want to pause until an event/task finishes.
//
// SYNTAX:
//   co_await <awaitable>;
//
// BEST USE CASE:
//   Networking — await a download to finish, then resume
//   and use the downloaded data. No blocking. No threads.
//
// ------ EXAMPLE: co_await with suspend_always -------------

// coro[int] do_work() {
//     std::cout << "Doing first thing..." << std::endl;
//     co_await std::suspend_always{};   // suspension point #1
//
//     std::cout << "Doing second thing..." << std::endl;
//     co_await std::suspend_always{};   // suspension point #2
//
//     std::cout << "Doing third thing..." << std::endl;
//     // coroutine ends naturally here
// }
//
// int main() {
//     auto task = do_work();   // coroutine suspended on creation
//
//     task.resume();  // runs → prints "first"  → hits point #1 → PAUSE
//     task.resume();  // runs → prints "second" → hits point #2 → PAUSE
//     task.resume();  // runs → prints "third"  → reaches END   → DONE
//     task.resume();  // ⚠ DISASTER! Resuming a completed coroutine
//
//     std::cout << "Done!" << std::endl;
//     return 0;
// }
//
// EXECUTION FLOW (do_work):
//
//  do_work() called
//       │
//       ▼
//  [PAUSED at creation — initial_suspend]
//       │
//  task.resume() #1
//       │
//       ▼
//  "Doing first thing..."
//  co_await suspend_always → [PAUSED at point #1]
//       │
//  task.resume() #2
//       │
//       ▼
//  "Doing second thing..."
//  co_await suspend_always → [PAUSED at point #2]
//       │
//  task.resume() #3
//       │
//       ▼
//  "Doing third thing..."
//  [COROUTINE DONE]
//       │
//  task.resume() #4  →  ⚠ UNDEFINED BEHAVIOR
//
// ============================================================
// co_yield vs co_return vs co_await — SIDE BY SIDE
// ============================================================
//
//  +-------------+---------+------------------+-------------+
//  |             | Pauses? | Returns value?   | Ends coro?  |
//  +-------------+---------+------------------+-------------+
//  | co_yield v  |  YES    | YES (v to caller)|  NO         |
//  | co_return v |  NO     | YES (final value)|  YES        |
//  | co_await x  |  YES    | NO (waits only)  |  NO         |
//  +-------------+---------+------------------+-------------+
//
// ============================================================
// ⚠ THE GOLDEN RULE
// ============================================================
//
//   NEVER resume a coroutine that has finished (reached
//   co_return or run off the end of the function body).
//
//   Doing so = Undefined Behavior:
//     - might crash
//     - might return garbage
//     - might silently corrupt memory
//
//   ALWAYS track whether the coroutine is done:
//     handle.done()  →  returns true if coroutine has completed
//
//   Safe resume pattern:
//     if (!handle.done()) {
//         handle.resume();
//     }
//
// ============================================================
// NOTE ON RUNNING THIS CODE
// ============================================================
//
//   C++20 does NOT provide a ready-made generator<T> or
//   coro[T] type. You must either:
//
//   1. Build your own coroutine type (promise_type + handle)
//      → See: cpp20_coroutines_notes.cpp (previous lecture notes)
//
//   2. Use a third-party library:
//      → cppcoro:  https://github.com/lewissbaker/cppcoro
//
//   3. Use C++23 which adds std::generator<T> built-in
//
// ============================================================