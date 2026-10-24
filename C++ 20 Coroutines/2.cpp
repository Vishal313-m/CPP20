/* Coroutine WORKFLOW*/

1. Normal Function Call Flow

Code:

#include <iostream>

void func1()
{
    std::cout << "func1\n";
}

void func2()
{
    double a = 56;
    double b = 100;

    func1();
}

void func3()
{
    func2();
}

int main()
{
    func3();
}
Step 1 : main() starts

Call Stack:

+--------+
| main   |
+--------+
Step 2 : main() calls func3()
+--------+
| func3  |
+--------+
| main   |
+--------+
Step 3 : func3() calls func2()
+--------+
| func2  |
+--------+
| func3  |
+--------+
| main   |
+--------+
Step 4 : func2() calls func1()
+--------+
| func1  |
+--------+
| func2  |
+--------+
| func3  |
+--------+
| main   |
+--------+
Step 5 : func1() returns
+--------+
| func2  |
+--------+
| func3  |
+--------+
| main   |
+--------+
Step 6 : func2() returns
+--------+
| func3  |
+--------+
| main   |
+--------+
Step 7 : func3() returns
+--------+
| main   |
+--------+
Step 8 : main() exits
EMPTY

This is called stack unwinding.















2. What Changes With Coroutines?

Suppose func2 becomes a coroutine.

Task func2()
{
    double a = 56;
    double b = 100;

    co_await something;   // Suspension point

    func1();
}

At co_await execution pauses.

3. Coroutine Frame

When suspension happens:

double a = 56;
double b = 100;

must not be lost.

C++ stores them in a special memory block:
---------------------------------------------------------------
Coroutine Frame
----------------
a = 56
b = 100

resume point:
co_await something

handle
------------------------------------------------------------------------------
Usually allocated on the heap.

4. Execution Before Suspension

Call sequence:

main()
{
    func3();
}
func3()
{
    func2();
}

Stack:

+--------+
| func2  |
+--------+
| func3  |
+--------+
| main   |
+--------+
5. Suspension Happens

Execution reaches:

co_await something;

State is saved:

Coroutine Frame
---------------
a = 56
b = 100

Resume after:
co_await something

Now func2 leaves the stack.

Stack becomes:

+--------+
| func3  |
+--------+
| main   |
+--------+

func3 finishes:

+--------+
| main   |
+--------+
6. Important Difference

Normal function:

main
 └─ func3
     └─ func2
         └─ func1

returns immediately

Coroutine:

main
 └─ func3
     └─ func2

func2 pauses

func2 removed from stack

main continues running
7. Resuming The Coroutine

Later:

handle.resume();

C++ restores the saved frame:

Coroutine Frame
---------------
a = 56
b = 100

resume point:
co_await something

Stack becomes:

+--------+
| func2  |
+--------+
| main   |
+--------+

Execution continues immediately after suspension point.

8. Then func1() Gets Called
Task func2()
{
    double a = 56;
    double b = 100;

    co_await something;

    func1();
}

Stack:

+--------+
| func1  |
+--------+
| func2  |
+--------+
| main   |
+--------+

func1 returns:

+--------+
| func2  |
+--------+
| main   |
+--------+

func2 finishes:

+--------+
| main   |
+--------+

main finishes:

EMPTY
Complete Visualization
Normal Function
main
 |
 +--> func3
        |
        +--> func2
               |
               +--> func1

func1 returns
func2 returns
func3 returns
main returns
Coroutine
main
 |
 +--> func3
        |
        +--> func2

               |
               +--> SUSPEND
                     save state

func2 removed

func3 returns

main keeps running
      |
      +--> resume func2

               |
               +--> func1

func1 returns
func2 returns
main returns
Real C++20 Example
#include <coroutine>
#include <iostream>

struct Task
{
    struct promise_type
    {
        Task get_return_object()
        {
            return {};
        }

        std::suspend_never initial_suspend()
        {
            return {};
        }

        std::suspend_always final_suspend() noexcept
        {
            return {};
        }

        void return_void() {}

        void unhandled_exception() {}
    };
};

Task func2()
{
    int a = 56;
    int b = 100;

    std::cout << "Before suspend\n";

    co_await std::suspend_always{};

    std::cout << "After resume\n";
}

int main()
{
    func2();

    std::cout << "Main continues\n";
}

Conceptually:

Before suspend

func2 pauses

Main continues

resume()

After resume
Interview Point

A normal function's state lives on the stack and disappears when the function returns.

A coroutine's state is copied into a coroutine frame (usually on the heap), allowing the function to suspend and later resume from exactly the same point with all local variables preserved.

This is the core idea behind co_await, co_yield, and asynchronous programming in C++20.



