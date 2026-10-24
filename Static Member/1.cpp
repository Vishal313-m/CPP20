 /*  https://www.youtube.com/watch?v=Uwzfm3UJWww*/

What is static variable 
A static variable in C++ is a variable that is declared using the keyword static. The space for the static variable is allocated only one time 
and this is used for the entirety of the program. Once this variable is declared, it exists till the program executes.
 So, the lifetime of a static variable is the lifetime of the program.

When a variable is declared as static, space for it gets allocated for the lifetime of the program.
 Even if the function is called multiple times, space for the static variable is allocated only once and
  the value of the variable in the previous call gets carried through the next function call

#include <iostream>
#include <string>
using namespace std;

void demo()
{
	// static variable
	static int count = 0;
	cout << count << " ";

	// value is updated and
	// will be carried to next
	// function calls
	count++;
}

int main()
{
	for (int i = 0; i < 5; i++)
		demo();
	return 0;
}