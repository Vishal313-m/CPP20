/*Move semantics 

It is a bunch of features that really make up a mechanism.
We have to avoid throwing away those temporary objects that might contain the data that we actually need.
And using move semantics, we can avoid unnecessary copies.
So if we have our books container object, which is a temporary here, what we would really like to
do is to not throw away this temporary object, but we can steal data from it.
So for example, we can make our pointer here, go and point to the temporary object internal dynamic
array and we can tell it m items pointer to be a null pointer.
we can tell it to reset itself to null pointer so we can do something like this.
And then the memory location that was managed by this temporary object is going to belong to us.
When the temporary object dies, it is going to die alone.
It is not going to go with the data that we need.
The data is going to stay and we will be pointing to this data here.
This is the setup we want and we can achieve this using move semantics.
So by the time the temporary object dies, the data is going to be ours and we will have a situation
like this and we will have avoided unnecessary copies in our C plus plus program.


Move semantics in C++ refers to the ability to efficiently transfer resources from one object to another
 without creating unnecessary copies. In C++, move semantics are implemented through move constructors and move assignment operators,
  allowing the transfer of resources like dynamically allocated memory or unique ownership pointers. When an object is moved,
  its resources are "transferred" to another object, leaving the original object in a valid but unspecified state.
  This mechanism is particularly useful for optimizing performance by avoiding unnecessary deep copies of objects.


Move semantics in C++ refer to a set of rules and tools that enable the efficient transfer of resources from one object to another, 
instead of copying the data, when the object's lifetime expires

.
Move semantics involve using move constructors that take r-value references, allowing the direct transfer of resources between objects,
 reducing memory usage and improving performance

*/


/* Lvalues - Lvalues are - if you take address of expression and use at later time then it is a Lvalues */

/*Rvalues - Rvalues are temporary in nature , they only exist for a short time and are quickly destroyed by the system when no longer needed*/


/*
Lvalue
int x{5};

int y{10};

int z{20};
*/

/*
Rvalue

z=(x+y) ------
          15    <------ rvalue
        ------
*/

The result of (x+y) is stored in memory for a short time(transiently) before its assigned (copied to z).After the assignment , the memory is (x+y) is an __cpp_rvalue_references

/*

int *ptr1 = &(x+y)   //compiler error


int *ptr3 = &45 //compiler error


int *ptr4 = &x;  //x is a lVALUE SO WE CAN GRAB ITS ADDRESS




*/


/*  https://www.youtube.com/watch?v=SoJvhbQBOBw */