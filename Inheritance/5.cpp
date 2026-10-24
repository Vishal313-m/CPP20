/* Resurecting MEmbers  back in CONTEXT */


/*We have a base class person.
The engineer class is going to do private inheritance and it's going to really be selfish and say,
I am going to take everything I can from the upstream person class, but I am going to make everything
my own and nobody inherited from me will have access to these guys.
For example, M1 and M2 are going to be private to the engineer class starting here when the engineer class does private inheritance.
So any other class, for example, civil engineer which is inheriting from the engineer class in any
way, won't have access to the M1 and M2 member variables.
So suppose for some reason that we have this kind of setup here.
But again, we want some members to be accessible to other downstream classes.
Suppose, for example, we want the M1 member variable to still be accessible to downstream classes.
While we have done private inheritance, Cplusplus is really weird in a way.
It is going to work hard to really prevent you to do things and then give you a back door you can go
through to allow the same thing you were trying to avoid in the same place.*/


/* PErson.h*/

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





/* Person.cpp*/

#include "person.h"
#include <iostream>

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






/*Engineer.h*/


ifndef ENGINEER_H
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

    public : 
        using Person::add;

    protected : 
        using Person::get_full_name;
        using Person::get_age;
        using Person::get_address;
        //using Person::m_address; // Compiler error

        /* You shouldn't really try to resurrect something that is already private to an upstream class and that's what we are doing here.
      We are trying to resurrect a member which is private to person and this is not allowed in C plus plus. If we do that, we're going to get a compiler error.*/

    int get_contract_count() const{
        return contract_count;
    }
    
private : 
    int contract_count{0};
};

#endif // ENGINEER_H







/*Engineer.cpp*/

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






/*Civilengineer.h*/

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


        /* We have this method here, but if we do this, we're going to get a compiler error because this method is now private to the engineer class.
And why is it private to engineer?
Well, engineer is doing private inheritance that's going to eclipse everything to private level access by default.
So this member is private to engineer.
We are inheriting from engineer in civil engineer, and this is going to be already private to engineer and we can't resurrect this back.
So if you eclipse something using private inheritance, you want to be able to resurrect that to a more relaxed access level in downstream classes.
*/
	
private : 
    std::string m_speciality{"None"};

};

#endif // CIVIL_ENGINEER_H





/*Civilengineer.cpp*/


#include "person.h"
#include "engineer.h"
#include "civilengineer.h"
#include <iostream>

CivilEngineer::CivilEngineer()
{
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





/*  Main.cpp*/


#include <iostream>
#include "person.h"
#include "engineer.h"
#include "civilengineer.h"

int main(){

    

  
   
    return 0;
}