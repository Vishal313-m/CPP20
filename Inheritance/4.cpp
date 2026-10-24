/* Closing In On Private Inheritance */


/*Person.h*/

#ifndef PERSON_H
#define PERSON_H

#include <string>
#include <string_view>
class Person
{
    friend std::ostream& operator<<(std::ostream& , const Person& person);
public:
    Person() = default;
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
public:
    std::string m_full_name{"None"};
protected: 
    int m_age{0};
private : 
    std::string m_address{"None"};
};


#endif // PERSON_H






/* Person.cpp*/

#include "person.h"
#include <iostream>

Person::Person(std::string_view fullname, int age,
    const std::string address) 
        : m_full_name{fullname}, m_age{age},
            m_address{address}
{
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






/*Engineer.h*/

#ifndef ENGINEER_H
#define ENGINEER_H
#include "person.h"

//Engineer is doing private inheritance

class Engineer : private Person
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
    
private : 
    int contract_count{0};
};

#endif // ENGINEER_H







/* Engineer.cpp*/

#include "person.h"
#include "engineer.h"
#include <iostream>

Engineer::Engineer()
{
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







/* Civil Engineer.h */


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
    }
	
private : 
    std::string m_speciality{"None"};

};

#endif // CIVIL_ENGINEER_H






/* Civil Engineer.cpp*/


#include "person.h"
#include "engineer.h"
#include "civilengineer.h"
#include <iostream>

CivilEngineer::CivilEngineer()
{
}

std::ostream& operator<<(std::ostream& out, const CivilEngineer& operand){
    
    /*
     out << "CivilEngineer [Full name : " << operand.get_full_name() <<
                    ",age : " << operand.get_age() << 
                    //",address : " << operand.get_address() <<
                    ",contract_count : " << operand.contract_count <<
                    ",speciality : " << operand.m_speciality << "]";
                    */
                  
				
    return out;  
}


CivilEngineer::~CivilEngineer()
{
}







/* Main.cpp*/

#include <iostream>
#include "person.h"
#include "engineer.h"
#include "civilengineer.h"

int main(){

    Engineer eng1;

    CivilEngineer ce1;

    std::cout << "Done" << std::endl;// BeakPoint 



    /* ------ See Image ------ 
    we are going to hit our breakpoint in a minute.

  Once we hit our breakpoint, we are going to look at the local variables we have in the main function.
  We're going to see them on the left here.

If we expand, we're going to see that we have an engineer, one object, and we have a civil engineer, one object.

If we look at our engineer, we're going to see that we have a person part of this, which is our base class.

And if we look at civil engineer, it's also going to have a base of engineer. So any civil engineer is also going to be an engineer.

But the engineer, part of civil engineer also has a person part of it.

You should really see this information.
And the main message I want to convey here is that all these objects have a person.Part of them.

We just can't access that part in object of our inheriting classes because of the design decisions we have made.

    */

  


  /* 
  
  Now that you have seen this, and again, we are really working on this inheritance hierarchy.
The base class is person and it's going to have these member variables.
But if we do private inheritance from the person class, anything that is not public is going to be
eclipsed down to private level access.
So any other class that is going to inherit from our engineer class now is not going to have direct
access to these members because they are going to become private to this class here.
And that's the experience we have If we try to use any of this methods and members we get from engineer,
this is not going to work because they are now private to the engineer class.
This is the message here.
So the get full name get age methods are not going to work.
If we try to use the contract count member, which comes from the engineer class, that's not also going to work.
If we look in our header and try to use any of these members, we're going to get compiler errors because
all these guys are now private to the engineer class.

  */
   
    return 0;
}