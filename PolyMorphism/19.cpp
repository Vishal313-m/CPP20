
/* https://www.youtube.com/watch?v=sAZ6JvLPx_0&t=538s */


Vtable (Virtual Table):
The vtable is a lookup table that the compiler creates for each class that has virtual functions.
The vtable contains the addresses of the virtual functions that belong to that class.
The vtable is a static array that is shared among all objects of the same class.
The vtable allows the compiler to resolve virtual function calls at runtime by looking up the appropriate function address in the table.






Vptr (Virtual Pointer):
The vptr is a hidden pointer that the compiler adds to every object of a class that has virtual functions.
The vptr points to the vtable of the object's class.
When an object is created, the vptr is automatically initialized to point to the vtable of the object's class.
The vptr allows the object to access the appropriate vtable at runtime, enabling dynamic dispatch of virtual function calls.



The relationship between vtable and vptr can be summarized as follows:
The vtable is a static table of function pointers, one for each virtual function in the class.
The vptr is a hidden pointer within each object that points to the vtable of the object's class.
When a virtual function is called on an object, the object's vptr is used to look up the appropriate function pointer in the vtable, 
and the corresponding function is then called.




This mechanism allows for dynamic dispatch, where the correct version of a virtual function is called at runtime based on the actual type of the object,
 rather than the static type of the pointer or reference used to access the object.



