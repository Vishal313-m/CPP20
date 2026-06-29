
// https://www.youtube.com/watch?v=St0MNEU5b0o


In C++, std::move is a utility function that performs a cast to an rvalue reference.
 It allows you to explicitly convert an lvalue (a value that has a name and is stored in memory) to an rvalue (a temporary value that typically doesn't have a name) 
 so that it can be moved rather than copied during operations like assignments or function returns.


 std::move is just a static cast .



 int main()
 {
    //construct a new string . This uses copy constructor we have am 'rvalue' on the right and that will be copied into 'myString' using the copy constructor.


    std::string myString = "Copy Construct me ";
    std::string newValue;


    //std::move allows us to adopt or steal the value 

    //newValue = std::move(myString);

    //Could more explicitly call the static cast here to move the value of myString into newValue.


    newValue = static_cast<std::string&&>(myString);

    //Note here , myString is still some 'valid' value.
    //Maybe it stores 'nullptr' in the data in the string but it still exists

    std::cout<<myString<<std::endl;

    std::cout<<newValue<<std::endl;
    
     }
