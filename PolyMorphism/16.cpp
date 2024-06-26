/* typeid Operator  */ 

/* The typeid operator allows the type of an object to be determined at run time. */
/*  This is an operator we have in C++ we can use to ask a base pointer which kind of derived object it is managing at a given moment in time.*/

/* This is only going to work for polymorphic types. So if you don't have polymorphism going on in your inheritance hierarchy, this is only 
going to give you static binding results and type ID is going to return the name of a dynamic type if it can. And if it can't, it's going
to give you the static type that is a result of static binding in your inheritance hierarchy.*/


#include<iostream>
#include<typeinfo>

class DynamicBase{
public: 
    virtual void do_something(){
        std::cout << "DynamicBase::do_something() called" << std::endl;
    }
    virtual ~DynamicBase() = default;
};

class DynamicDerived : public DynamicBase{
public : 
    virtual void do_something() override{
        std::cout << "DynamicDerived::do_something() called" << std::endl;
    } 
    virtual ~DynamicDerived() = default;
};


class StaticBase{
     void do_something(){
        std::cout << "StaticBase::do_something() called" << std::endl;
    }
};

class StaticDerived : public StaticBase{
     void do_something() {
        std::cout << "StaticDerived::do_something() called" << std::endl;
    }    
};


int main(){

	//1. typeid with fundemental types : returns static type
    std::cout << "typeid(int) : " << typeid(int).name() << std::endl;
	if(typeid(22.2f) == typeid(float)){
		std::cout << "22.2f is a float" << std::endl;
	}else{
		std::cout << "22.2f is not float" << std::endl;
	}

	std::cout << "----------------" << std::endl;
	//2. typeid with references(polymorphic)
	std::cout << "Polymorphic references : " << std::endl;
	DynamicDerived dynamic_derived;
	DynamicBase& base_ref = dynamic_derived;
	std::cout << "Type of dynamic_derived : " << typeid(dynamic_derived).name() << std::endl;
	std::cout << "Type of base_ref : " << typeid(base_ref).name() << std::endl;


	std::cout << "----------------" << std::endl;

	//3. typeid with pointers(polymorphic)
	std::cout << "Polymorphic pointers : " << std::endl;
	DynamicBase * b_ptr = new DynamicDerived;
  
	std::cout << "Type of b_ptr : " << typeid(b_ptr).name() << std::endl; // static type
	
	//ATTENTION :
	//			 For pointers you have to dereference to see the dynamic type //
	std::cout << "Type of *b_ptr : " << typeid(*b_ptr).name() << std::endl;


	std::cout << "----------------" << std::endl;

	//4. type id with non polymorphic pointers and refs : We'll get static types
	//because we're using static binding, the default behavior
	std::cout << "Non polymorphic pointers and refs : " << std::endl;
	StaticBase * b_ptr_s = new StaticDerived;
	StaticDerived staticderived;
	StaticBase& static_base_ref {staticderived};
  
	std::cout << "Type of *b_ptr_s : " << typeid(*b_ptr_s).name() << std::endl;
	std::cout << "Type of static_base_ref : " << typeid(static_base_ref).name() << std::endl;
  

    delete b_ptr;
    delete b_ptr_s;
	

    return 0;
}

/*
Output

typeid(int) : i
22.2f is a float
----------------
Polymorphic references : 
Type of dynamic_derived : 14DynamicDerived
Type of base_ref : 14DynamicDerived
----------------
Polymorphic pointers : 
Type of b_ptr : P11DynamicBase
Type of *b_ptr : 14DynamicDerived
----------------
Non polymorphic pointers and refs : 
Type of *b_ptr_s : 10StaticBase
Type of static_base_ref : 10StaticBase




*/