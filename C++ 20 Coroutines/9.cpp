// ============================================================
//   C++20 — THIRD-PARTY COROUTINE TYPES (unique_generator)
// ============================================================
//
// WHY USE THIRD-PARTY TYPES?
//   Building your own coroutine type is complex and error-prone.
//   Libraries like cppcoro / unique_generator provide ready-made,
//   battle-tested types you can drop in and use immediately.
//
// RECOMMENDED LIBRARIES:
//   cppcoro  → https://github.com/lewissbaker/cppcoro
//              (reference implementation, complex to build)
//   "coro"   → https://github.com/lewissbaker/llvm
//              (header-only: just copy unique_generator.h into project)
//
// ============================================================
// unique_generator vs our custom generator — KEY DIFFERENCES
// ============================================================
//
//   Feature               | our generator<T>   | unique_generator<T>
//   ──────────────────────|─────────────────────|──────────────────────
//   operator()()          | YES (built-in)      | YES (added in header)
//   begin() / end()       | NO                  | YES (has iterator class)
//   Range-based for loop  | NO                  | YES ✅
//   Memory management     | Basic               | Manual lifetime wrapper
//   Move-only (unique)    | No                  | YES (move-only, no copy)
//   Ref type support      | No                  | YES (Ref, Value params)
//
// UNIQUE GENERATOR SIGNATURE:
//   template<class Ref, class Value = std::decay_t<Ref>>
//   class unique_generator { ... };
//
//   Usage: unique_generator<int>  → Ref=int, Value=int
//
// ============================================================
// HOW TO USE unique_generator IN YOUR PROJECT
// ============================================================
//
// Step 1: Copy unique_generator.h into your project folder
// Step 2: #include "unique_generator.h"
// Step 3: Change return types from generator<T> to unique_generator<T>
// Step 4: Use begin()/end() iterators OR operator()() to consume values
//
// ============================================================
// THREE WAYS TO CONSUME A unique_generator
// ============================================================
//
// WAY 1: operator()() — call like a function (resume + return value)
//   auto val = task();   // resumes coroutine, returns yielded value
//
// WAY 2: begin()/end() iterators — manual iterator loop
//   for (auto it = task.begin(); it != task.end(); ++it) {
//       std::cout << *it << std::endl;
//   }
//
// WAY 3: Range-based for loop — cleanest syntax
//   for (auto v : task) {
//       std::cout << v << std::endl;
//   }
//   (Works because unique_generator has begin() and end() methods)
//
// ============================================================
// FULL WORKING CODE
// ============================================================

#include <iostream>
#include "unique_generator.h"   // copy from: https://github.com/lewissbaker/llvm

// ── Coroutine functions — same as before, just change return type ──

// Fixed sequence
unique_generator<int> generate_numbers() {
    co_yield 10;
    std::cout << "After stop point #1" << std::endl;

    co_yield 20;
    std::cout << "After stop point #2" << std::endl;

    co_yield 30;
    std::cout << "After stop point #3" << std::endl;
}

// Infinite stream
unique_generator<int> infinite_number_stream(int start = 0) {
    auto value = start;
    for (int i = 0;; ++i) {
        co_yield value;
        ++value;
    }
}

// Range: yields [first, last)
unique_generator<int> range(int first, int last) {
    while (first != last) {
        co_yield first;
        ++first;
    }
}

int main() {
    // ── Example 1: generate_numbers via iterator loop ────────
    std::cout << "=== generate_numbers (iterator loop) ===" << std::endl;
    auto task1 = generate_numbers();

    for (auto it = task1.begin(); it != task1.end(); ++it) {
        std::cout << "value: " << *it << std::endl;
    }
    // Output: value: 10
    //         After stop point #1
    //         value: 20
    //         After stop point #2
    //         value: 30
    //         After stop point #3

    std::cout << std::endl;

    // ── Example 1b: generate_numbers via operator() ──────────
    std::cout << "=== generate_numbers (operator()) ===" << std::endl;
    auto task1b = generate_numbers();

    std::cout << "value: " << task1b() << std::endl;   // 10
    std::cout << "value: " << task1b() << std::endl;   // After #1, then 20
    std::cout << "value: " << task1b() << std::endl;   // After #2, then 30

    std::cout << std::endl;

    // ── Example 2: infinite stream — print first 10 ──────────
    std::cout << "=== infinite_number_stream ===" << std::endl;
    auto task2 = infinite_number_stream(0);

    for (size_t i = 0; i < 10; ++i) {
        std::cout << "value: " << task2() << std::endl;
    }
    // Output: 0, 1, 2, ... 9

    std::cout << std::endl;

    // ── Example 3: range via range-based for loop ────────────
    std::cout << "=== range(0, 5) via range-based for ===" << std::endl;
    auto task3 = range(0, 5);

    for (auto v : task3) {
        std::cout << "value: " << v << std::endl;
    }
    // Output: 0, 1, 2, 3, 4

    std::cout << "Done!" << std::endl;
    return 0;
}

// ============================================================
// HOW unique_generator's ITERATOR WORKS INTERNALLY
// ============================================================
//
//   begin() {
//       coro_.resume();        // resume coroutine once
//       return iterator{coro_};
//   }
//
//   iterator::operator*() {
//       return coro_.promise().get();   // return current value
//   }
//
//   iterator::operator++() {
//       coro_.promise().clear_value();  // clear old value
//       coro_.resume();                 // advance to next yield
//       return *this;
//   }
//
//   operator==(it, sentinel) {
//       return it.coro_.done();   // stop when coroutine is done
//   }
//
//   So the range-based for loop:
//     for (auto v : task3) { ... }
//   expands to:
//     auto it  = task3.begin();    // resumes once
//     auto end = task3.end();      // returns sentinel{}
//     while (it != end) {          // checks coro_.done()
//         auto v = *it;            // reads promise().get()
//         ...
//         ++it;                    // resumes to next yield
//     }
//
// ============================================================
// unique_generator.h — KEY SECTIONS SUMMARY
// ============================================================
//
//   promise_type:
//     get_return_object()  → creates unique_generator from handle
//     initial_suspend()    → suspend_always (pause on call)
//     final_suspend()      → suspend_always (pause at end)
//     yield_value(Ref ref) → stores value, returns suspend_always
//     return_void()        → called when coroutine body ends
//     unhandled_exception()→ rethrows to caller
//     get()                → returns reference to stored value
//
//   manual_lifetime<T>:
//     A union-based wrapper for safe uninitialized storage.
//     construct() / destruct() / get() manage the stored value.
//     Used inside promise_type instead of a plain T member.
//
//   iterator class:
//     Enables begin()/end() and range-based for loops.
//     Uses sentinel{} as the end marker (not another iterator).
//
//   operator()():
//     coro_.resume() + return coro_.promise().get()
//     Added to the header so we can use functor-style calls.
//
// ============================================================
// COMPILER SETUP
// ============================================================
//
//   GCC 10/11  → compile with:  -fcoroutines
//   MSVC 2019+ → no extra flag needed
//   Online     → https://wandbox.org
//
// ============================================================
// SUMMARY: WHEN TO USE WHAT
// ============================================================
//
//   Build your own coroutine type:
//     → Only if you really need a custom behavior
//     → Only if you fully understand the infrastructure
//     → Good for learning (what we did in previous lectures)
//
//   Use unique_generator / cppcoro / C++23 std::generator:
//     → Production code
//     → When you just need a working generator
//     → Saves time, less error-prone
//
//   C++23 note:
//     std::generator<T> is NOW built into C++23 standard library.
//     No third-party needed if you're on C++23!
//     Usage: #include <generator>  →  std::generator<int>
//
// ============================================================