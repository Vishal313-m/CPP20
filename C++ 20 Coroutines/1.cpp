Coroutines are a generalization of functions in C++
They are designed to make writing asynchronous code much easier


/* FUNCTIONS*/
Now, if we look back at functions,
 they can really do two things 
 -- >You can call a function
 --> a function can optionally return something.


 But in C++ 20, the features of the function have been expanded.


 /* Coroutine*/
 --> Can be called 
 -- > Can return Something
 --- > Can be Paused 
 ----> Can be resumed 


 Now we will have the ability to pause a function and we can resume it to make it keep doing whatever
it was doing before we paused it.
And we are able to do this because of the features that Coroutines bring in place.






/*

1. What is a Coroutine?

Before C++20, a function could only do two things:

int add(int a, int b)
{
    return a + b;
}

When called:

int result = add(10, 20);

The function:

Starts executing
Runs until completion
Returns a value
Its stack frame is destroyed

A normal function cannot pause and continue later.

2. What C++20 Adds

Coroutines allow a function to:

Start execution
Pause execution
Save its state
Resume later
Continue from where it stopped

Think of it like watching a movie:

Play
Pause
Resume
Pause
Resume
Finish

Normal functions are:

Play
Finish

only.

3. Example

Imagine a coroutine:

Generator numbers()
{
    co_yield 10;
    co_yield 20;
    co_yield 30;
}

Execution:

First resume
10

Coroutine pauses here:

co_yield 10;
Second resume

Continues from previous position:

20

Pauses again.

Third resume

Continues again:

30

Pauses.

Fourth resume

Coroutine finishes.

4. What Happens During Pause?

Suppose we have:

Generator example()
{
    int x = 10;

    co_yield x;

    x++;

    co_yield x;
}

When first paused:

co_yield x;

C++ stores:

x = 10

and remembers:

Resume from here

This stored information is called the Coroutine Frame.

5. Coroutine Frame

A Coroutine Frame contains:

Local Variables
Parameters
Current Execution Position
Promise Object
Coroutine State

Example:

Generator example(int a)
{
    int x = 10;

    co_yield x;

    x++;
}

The frame stores:

a = ?
x = 10
Current Line = after first co_yield

so execution can continue later.

6. Why Heap Allocation?

Normal functions use the stack:

main()
 └── func()

When func() returns:

Stack frame destroyed

But a coroutine may survive after returning control to the caller.

Therefore its state must be stored somewhere that survives:

Heap Memory

This memory block is the Coroutine Frame.

7. Coroutine Handle

C++ gives a special object:

std::coroutine_handle<>

This handle acts like a remote control.

handle.resume();

continues execution.

handle.destroy();

frees resources.

Think:

Coroutine Frame
      ↑
      |
Coroutine Handle

The handle knows where the coroutine state is stored.

8. Coroutine Workflow

Suppose:

Generator numbers()
{
    co_yield 10;
    co_yield 20;
    co_yield 30;
}

Execution flow:

Call coroutine
      |
      V
co_yield 10
      |
      V
Pause
      |
      V
Resume
      |
      V
co_yield 20
      |
      V
Pause
      |
      V
Resume
      |
      V
co_yield 30
      |
      V
Pause
      |
      V
Resume
      |
      V
Finish
9. Lazy Evaluation

Coroutines can generate values only when needed.

Example:

Generator<int> numbers()
{
    int i = 0;

    while(true)
    {
        co_yield i++;
    }
}

Nothing is generated until requested:

auto g = numbers();

g.next(); // 0
g.next(); // 1
g.next(); // 2

Values are produced on demand.

This is called lazy computation.

10. Similarity with Ranges

In C++20 ranges:

auto values =
    std::views::iota(1)
    | std::views::filter([](int x)
      {
          return x % 2 == 0;
      });

Numbers are generated only when needed.

Coroutines can implement the same idea:

co_yield value;

Generate one value → pause → generate next value later.

11. Important Point

C++20 does not provide a built-in Generator type.

Instead it provides:

<coroutine>

which contains the coroutine infrastructure:

std::coroutine_handle
promise_type
co_await
co_yield
co_return

You must build your own coroutine type or use a library that already provides one.

Think of C++20 as providing:

Engine

but not the complete car.

You build the car (Generator, Task, Async Task, etc.) using that engine.

Summary

Coroutines are special C++20 functions that can:

Start execution
Pause using co_yield or co_await
Save their state in a coroutine frame
Resume later using a coroutine handle
Perform lazy computations
Simplify asynchronous programming

The key idea is:

Normal Function:
Start → Run → Return

Coroutine:
Start → Pause → Resume
      → Pause → Resume
      → Finish

This ability to pause and continue later while preserving local variables and execution state is what makes C++20 Coroutines powerful.

Memory almost full
Once memory is full, responses will feel less personalized. Upgrade to expand memory, or manage existing memories.
Manage
Get more















*/

