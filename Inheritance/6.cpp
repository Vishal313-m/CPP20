/* DEFAULT CONSTRUCTORS WITH INHERITANCE */



/*PErson.h */

#ifndef PERSON_H
#define PERSON_H

#include <string>
#include <string_view>
class Person
{
    friend std::ostream& operator<<(std::ostream& , const Person& person);
public:
    Person() ;
    Person(std::string_view fullname,int age,
    const std::string address);
    ~Person();
    
    //Getters
    std::string get_full_name()const{
        return m_full_name;
    }
    
    int get_age()const{
        return m_age;
    }
    
    std::string get_address()const{
        return m_address;
    }


    int add(int a, int b) const{
        return a + b ;
    }

    int add(int a, int b, int  c) const{
        return a + b + c;
    }

    void do_something() const;
public:
    std::string m_full_name{"None"};
protected: 
    int m_age{0};
private : 
    std::string m_address{"None"};
};


#endif // PERSON_H







/*Person.cpp*/

#include "person.h"
#include <iostream>

Person::Person(){
    std::cout << "Default constructor for Person called..." << std::endl;
}

Person::Person(std::string_view fullname, int age,
    const std::string address) 
        : m_full_name{fullname}, m_age{age},
            m_address{address}
{
}

void Person::do_something() const{
    std::cout << "Hello" << std::endl;
}

std::ostream& operator<<(std::ostream& out , const Person& person){
   out << "Person [Full name :" << person.get_full_name() <<
                      ", Age:" << person.get_age() <<
                      ", Address:" << person.get_address() << "]";
  return out;
}


Person::~Person()
{
}






/* Engineer.h */

#ifndef ENGINEER_H
#define ENGINEER_H
#include "person.h"

class Engineer : public Person
{
friend std::ostream& operator<<(std::ostream& out , const Engineer& operand);
public:
    Engineer();
    ~Engineer();
    
    void build_something(){
        m_full_name = "John Snow"; // OK
        m_age = 23; // OK
        //m_address = "897-78-723"; Compiler error
    }

    int get_contract_count() const{
        return contract_count;
    }
    
private : 
    int contract_count{0};
};

#endif // ENGINEER_H









/* Engineer.cpp */ 


#include "person.h"
#include "engineer.h"
#include <iostream>

Engineer::Engineer()
{
    std::cout << "Default constructor for Engineer called..." << std::endl;
}

std::ostream& operator<<(std::ostream& out , const Engineer& operand){
     out << "Engineer [Full name : " << operand.get_full_name() <<
                    ",age : " << operand.get_age() << 
                    ",address : " << operand.get_address() <<
                    ",contract_count : " << operand.contract_count << "]";
    return out;   
}


Engineer::~Engineer()
{
}









/* CivilEngineer.h */ 


#ifndef CIVIL_ENGINEER_H
#define CIVIL_ENGINEER_H

#include "engineer.h"
class CivilEngineer : public Engineer
{
    friend std::ostream& operator<<(std::ostream&, const CivilEngineer& operand);
public:
    CivilEngineer();
    ~CivilEngineer() ;
    
    void build_road(){
        //get_full_name(); // Compiler error
        ///m_full_name = "Daniel Gray"; //Compiler error
        //m_age = 45; // Compiler error

        add(10,2);
        add(10,2,4);
    }

    public : 
        //using Person::do_something; // Compiler error
	
private : 
    std::string m_speciality{"None"};

};

#endif // CIVIL_ENGINEER_H







/* CivilEngineer.cpp */


#include "person.h"
#include "engineer.h"
#include "civilengineer.h"
#include <iostream>

CivilEngineer::CivilEngineer()
{
    std::cout << "Default constructor called for CivilEnginner ... " << std::endl;
}

std::ostream& operator<<(std::ostream& out, const CivilEngineer& operand){
     out << "CivilEngineer [Full name : " << operand.get_full_name() <<
                    ",age : " << operand.get_age() << 
                    ",address : " << operand.get_address() <<
                    ",contract_count : " << operand.get_contract_count() <<
                    ",speciality : " << operand.m_speciality << "]";
                    		
    return out;  
}


CivilEngineer::~CivilEngineer()
{
}






/* main.cpp */

#include <iostream>
#include "person.h"
#include "engineer.h"
#include "civilengineer.h"

int main(){

    CivilEngineer ce1;

    return 0;
}



/* 
OUTPUT

"Default constructor for Person called...

Default constructor for Engineer called...


"Default constructor called for CivilEnginner ... 


*/




/*
I am doing this so that I just have a point where I can put my break point.

Some ides are not going to like it if you put a break point at an empty line.

So that's why I am doing it here and I am going to try and debug and really show you that we have an

engineer and a person part in us.

I have shown you this before, but it doesn't really hurt to repeat this because debugging is cool.

It's going to give you more information than you could ever ask for.

We're going to hit our break point here, and if we look in our locals, we're going to have our civil

engineer object, which is in scope now.

And you see that we have an engineer, part of us, and the engineer has a person part in it.

So what the compiler is really going to do when it is building a civil engineer object, it is going

to build the most basic part of us, so it is going to build the person part.

After that, it is going to build an engineer layer on top of that and then it is going to build the

civil engineer layer.


*/