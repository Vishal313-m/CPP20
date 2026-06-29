/* FUNCTION HIDING  */

/*
IMP 
FUNCTION OVERLOADING   = Jab ek class hai or do function hai - naam same , argument aalag aalag usko bolte hai function overloading 

FUNCTION OVERRIDING    = Do class hai parent or child  , function ka naam same , argumment same , usko bolte hai function overriding 

Funtion hiding         = Do class hai parent or child  , function ka naam same , argumment aalag aalag , usko bolte hai function hiding 


*/
/* DEFINITION OF FUNCTION HIFDING -  If you are using any function name in derived class , it will hide all the functions coming from the base class with similar name  */


#include <iostream> 
using namespace std; 

class Base { 
public: 
	int fun()
    { 
        cout << "Base::fun() called";
    } 


	int fun(int i) 
    { 
        cout << "Base::fun(int i) called"; 
     } 
}; 

class Derived : public Base { 
public: 
	int fun()
    {
        cout << "Derived::fun() called"; 
    } 
}; 

// Driver Code 
int main() 
{ 
	Derived d; 
	d.fun(5); // Compiler Error 
	return 0; 
}

/*
Output
prog.cpp: In function ‘int main()’:
prog.cpp:20:12: error: no matching function for call to ‘Derived::fun(int)’
    d.fun(5); // Compiler Error
           ^
prog.cpp:13:9: note: candidate: int Derived::fun()
    int fun() { cout << "Derived::fun() called"; }
        ^
prog.cpp:13:9: note:   candidate expects 0 arguments, 1 provided
*/


/*------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

/* Ways to Access that d.fun(5); this func in derived class*/
/*Method 1  Scope resolution operator */

#include <iostream> 
using namespace std; 

class Base { 
public: 
	void  fun()
    { 
        cout << "Base::fun() called";
    } 


	void  fun(int i) 
    { 
        cout << "Base::fun(int i) called"; 
     } 
}; 

class Derived : public Base { 
public: 
	void fun()
    {
        cout << "Derived::fun() called"; 
    } 
}; 

// Driver Code 
int main() 
{ 
	Derived d; 
	d.Base::fun(5);
	return 0; 
}

/*
OUTPUT 
Base::fun(int i) called
*/

/*Method 2 WE CAN USE -  USING */

#include <iostream> 
using namespace std; 

class Base { 
public: 
	void  fun()
    { 
        cout << "Base::fun() called";
    } 


	void  fun(int i) 
    { 
        cout << "Base::fun(int i) called"; 
     } 
}; 

class Derived : public Base { 
public: 
     using Base::fun;
     void fun()
    {
        cout << "Derived::fun() called"; 
    } 
}; 

// Driver Code 
int main() 
{ 
	Derived d; 
	d.fun(5); 
	return 0; 
}

