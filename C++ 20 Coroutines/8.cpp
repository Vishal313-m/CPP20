// ============================================================
//   C++20 — CUSTOM GENERATOR COROUTINE TYPE (Complete Notes)
// ============================================================
//
// GOAL:
//   Build a reusable generator<T> coroutine type that:
//   - Works for any type T (template)
//   - Lazily generates values one at a time via co_yield
//   - Can be called like a function using operator()
//
// "LAZY" means: values are computed ON DEMAND, only when
//   you ask for them — not all at once upfront.
//
// ============================================================
// NEW CONCEPT: FUNCTOR (operator() overload)
// ============================================================
//
//   A functor is a struct/class whose objects can be called
//   like functions because operator() is defined.
//
//   T operator()() {
//       m_handle.resume();                   // resume coroutine
//       return m_handle.promise().m_value;   // return yielded value
//   }
//
//   Usage:
//     auto gen = generate_numbers();
//     int val = gen();    // calls operator(), resumes, returns value
//
// ============================================================
// FULL GENERATOR TEMPLATE
// ============================================================

#include <iostream>
#include <cassert>
#include <coroutine>

// ------------------------------------------------------------
// The generator<T> coroutine type (reusable for any type T)
// ------------------------------------------------------------
template <typename T>
struct generator {

    // --------------------------------------------------------
    // promise_type — controls coroutine lifecycle
    // --------------------------------------------------------
    struct promise_type {

        T m_value;   // stores the co_yield-ed value

        // Returns generator object to the caller
        auto get_return_object() { return generator{ *this }; }

        // Pause on first call (before body runs)
        auto initial_suspend() noexcept { return std::suspend_always{}; }

        // Pause at end (don't auto-destroy, we destroy in destructor)
        auto final_suspend()   noexcept { return std::suspend_always{}; }

        // Re-throw any coroutine exception to the caller
        void unhandled_exception() noexcept {
            std::rethrow_exception(std::current_exception());
        }

        // Called by co_yield v; — stores value and suspends
        auto yield_value(T const& v) {
            m_value = v;
            return std::suspend_always{};
        }

        // Called implicitly when coroutine body ends (no co_return)
        void return_void() {}
    };

    // --------------------------------------------------------
    // Constructor — builds from promise reference
    // --------------------------------------------------------
    explicit generator(promise_type& p)
        : m_handle(std::coroutine_handle<promise_type>::from_promise(p)) {}

    // Default constructor (for default-initialized generators)
    generator() = default;

    // --------------------------------------------------------
    // Destructor — frees heap memory
    // --------------------------------------------------------
    ~generator() {
        if (m_handle) {
            m_handle.destroy();
        }
    }

    // --------------------------------------------------------
    // operator() — makes generator a FUNCTOR
    // Call gen() to resume and get next value
    // --------------------------------------------------------
    T operator()() {
        assert(m_handle != nullptr);     // safety check
        m_handle.resume();               // resume coroutine
        return m_handle.promise().m_value;  // return yielded value
    }

private:
    std::coroutine_handle<promise_type> m_handle = nullptr;
};

// ============================================================
// THREE COROUTINE FUNCTIONS USING generator<T>
// ============================================================

// ------------------------------------------------------------
// 1. Fixed sequence: yields 10, 20, 30 then ends
// ------------------------------------------------------------
generator<int> generate_numbers() {
    co_yield 10;
    std::cout << "After stop point #1" << std::endl;

    co_yield 20;
    std::cout << "After stop point #2" << std::endl;

    co_yield 30;
    std::cout << "After stop point #3" << std::endl;
    // implicit co_return; here → return_void() called
}

// ------------------------------------------------------------
// 2. Infinite stream: yields 0, 1, 2, 3 ... forever
// ------------------------------------------------------------
generator<int> infinite_number_stream(int start = 0) {
    auto value = start;
    for (int i = 0;; ++i) {   // infinite loop — safe because co_yield pauses
        co_yield value;
        ++value;
    }
}

// ------------------------------------------------------------
// 3. Range generator: yields values from first to last-1
// ------------------------------------------------------------
generator<int> range(int first, int last) {
    while (first != last) {
        co_yield first;
        ++first;
    }
    // implicit co_return; when loop ends
}

// ============================================================
// main — using all three generators
// ============================================================
int main() {
    std::cout << std::boolalpha;

    // ────────────────────────────────────────────────────────
    // Example 1: generate_numbers()
    // ────────────────────────────────────────────────────────
    std::cout << "=== generate_numbers ===" << std::endl;
    auto task1 = generate_numbers();   // PAUSED (initial_suspend)

    std::cout << "value: " << task1() << std::endl;   // resume → 10 → PAUSE
    // Output: After stop point #1  ← NO! prints only when resumed PAST yield
    // Actually: "value: 10" (stop point #1 prints on NEXT resume)

    std::cout << "value: " << task1() << std::endl;   // resume → "After #1" → 20 → PAUSE
    std::cout << "coro done: " << task1.m_handle.done() << std::endl;
    // Output: After stop point #1 \n value: 20 \n coro done: false

    std::cout << "value: " << task1() << std::endl;   // resume → "After #2" → 30 → PAUSE
    std::cout << "coro done: " << task1.m_handle.done() << std::endl;
    // Output: After stop point #2 \n value: 30 \n coro done: false

    task1();   // resume → "After #3" → hits end → return_void() → DONE
    std::cout << "coro done: " << task1.m_handle.done() << std::endl;
    // Output: After stop point #3 \n coro done: true

    std::cout << std::endl;

    // ────────────────────────────────────────────────────────
    // Example 2: infinite_number_stream — print first 10
    // ────────────────────────────────────────────────────────
    std::cout << "=== infinite_number_stream ===" << std::endl;
    auto task2 = infinite_number_stream(0);   // starts at 0

    for (size_t i = 0; i < 10; ++i) {
        std::cout << "value: " << task2() << std::endl;
    }
    // Output: 0, 1, 2, 3 ... 9

    // Could loop forever: for(;;) { task2(); } — Ctrl+C to stop

    std::cout << std::endl;

    // ────────────────────────────────────────────────────────
    // Example 3: range(0, 5) — prints 0,1,2,3,4
    // ────────────────────────────────────────────────────────
    std::cout << "=== range(0, 5) ===" << std::endl;
    auto task3 = range(0, 5);

    for (size_t i = 0; i < 5; ++i) {
        std::cout << "value: " << task3() << std::endl;
    }
    task3();   // this resume causes coroutine to hit end → return_void()
    std::cout << "coro done: " << task3.m_handle.done() << std::endl;
    // Output: 0,1,2,3,4 then coro done: true

    std::cout << "Done!" << std::endl;
    return 0;
}

// ============================================================
// HOW operator() WORKS — STEP BY STEP
// ============================================================
//
//   T operator()() {
//       assert(m_handle != nullptr);        // don't call on null handle
//       m_handle.resume();                  // wake up the coroutine
//       return m_handle.promise().m_value;  // read stored yield value
//   }
//
//   task1()
//    │
//    ├─► assert handle is valid
//    ├─► m_handle.resume() → coroutine runs until next co_yield
//    │     co_yield 10 → yield_value(10) → m_value = 10 → PAUSE
//    └─► return m_handle.promise().m_value  →  returns 10
//
// ============================================================
// EXECUTION FLOW DIAGRAM (generate_numbers)
// ============================================================
//
//  main()                       generate_numbers() coroutine
//  ──────────────────           ────────────────────────────
//
//  auto task1 = generate_numbers()
//    initial_suspend() ───────► [PAUSED — before body]
//
//  task1() ─────────────────► resume
//                              co_yield 10
//                                yield_value(10) → m_value=10
//                              [PAUSED]
//  returns 10
//
//  task1() ─────────────────► resume
//                              "After stop point #1"
//                              co_yield 20
//                                yield_value(20) → m_value=20
//                              [PAUSED]
//  returns 20 | done: false
//
//  task1() ─────────────────► resume
//                              "After stop point #2"
//                              co_yield 30
//                                yield_value(30) → m_value=30
//                              [PAUSED]
//  returns 30 | done: false
//
//  task1() ─────────────────► resume
//                              "After stop point #3"
//                              [END] return_void() called
//                              final_suspend() → [DONE]
//  done: true
//
//  ~generator() → m_handle.destroy() → heap freed
//
// ============================================================
// LAZY GENERATION EXPLAINED
// ============================================================
//
//   EAGER (normal function):
//     Computes ALL values at once, stores in vector, returns.
//     Memory: O(N)
//
//   LAZY (generator coroutine):
//     Computes ONE value at a time, only when you call it.
//     Memory: O(1) — only the current value is held
//     Infinite streams are ONLY possible with lazy generation.
//
//   Example: infinite_number_stream
//     You CAN'T return a vector of all integers.
//     But you CAN yield them one by one as needed. ✅
//
// ============================================================
// SUMMARY TABLE: generator<T> vs CoroType (previous lecture)
// ============================================================
//
//   Feature              | CoroType (basic)    | generator<T>
//   ─────────────────────|─────────────────────|──────────────
//   Template             | No (int only)       | Yes (any T)
//   operator() functor   | No (manual resume)  | Yes (gen())
//   Handle is private    | No (public)         | Yes (private)
//   Null handle check    | No                  | Yes (assert)
//   Default constructor  | No                  | Yes
//   Use case             | Learning/demo       | Real-world use
//
// ============================================================
// COMPILER SETUP REMINDER
// ============================================================
//
//   GCC 10/11  →  compile with: -fcoroutines
//   MSVC 2019+ →  no extra flag
//   Online     →  https://wandbox.org
//
// ============================================================