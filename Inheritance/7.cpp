/* CUSTOM CONSTRUCTORS WITH INHERITANCE */



/* Person.h */

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
    std::string_view address);
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







/* Person.cpp */

#include "person.h"
#include <iostream>

Person::Person(){
    std::cout << "Default constructor for Person called..." << std::endl;
}

Person::Person(std::string_view fullname, int age,
    std::string_view address) 
        : m_full_name{fullname}, m_age{age},
            m_address{address}
{
        std::cout << "Custom constructor for Person called..." << std::endl;
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
    Engineer(std::string_view fullname,int age,
    std::string_view address,int contract_count);
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







/*Engineer.cpp */


#include "person.h"
#include "engineer.h"
#include <iostream>

Engineer::Engineer()
{
    std::cout << "Default constructor for Engineer called..." << std::endl;
}

Engineer::Engineer(std::string_view fullname,int age,
    std::string_view address,int contract_count_param) : 
Person(fullname,age,address),contract_count(contract_count_param)
{
    std::cout << "Custom constructor for Engineer called..." << std::endl;
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
    CivilEngineer(std::string_view fullname,int age,
    std::string_view address,int contract_count, std::string_view speciality);
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



CivilEngineer::CivilEngineer(std::string_view fullname,int age,
    std::string_view address,int contract_count, std::string_view speciality)
     : Engineer(fullname,age,address,contract_count), m_speciality(speciality)
{
    std::cout << "Custom constructor called for CivilEnginner ... " << std::endl;
}

/*
//BAD : Compiler error - m_address is private to person
CivilEngineer::CivilEngineer(std::string_view fullname,int age,
    std::string_view address,int contract_count, std::string_view speciality)
{
    m_full_name = fullname;
    m_age = age;
    //m_address = address;
    m_speciality = speciality;
    std::cout << "Custom constructor called for CivilEnginner ... " << std::endl;
}
*/

/*
CivilEngineer::CivilEngineer(std::string_view fullname,int age,
    std::string_view address,int contract_count, std::string_view speciality)
    :m_full_name(fullname),m_age(age),m_address(address),m_speciality(speciality)
{

    std::cout << "Custom constructor called for CivilEnginner ... " << std::endl;
}
*/

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






/*main.cpp*/


include "person.h"
#include "engineer.h"
#include "civilengineer.h"

int main(){
    /*
	Person person1("John Snow",27,"Winterfell Cold 33St#75");
	std::cout << "person1 : " << person1 << std::endl;
    */

   /*
	std::cout << "---------------------"<< std::endl;
	Engineer eng1("Daniel Gray",41,"Green Sky Oh Blue 33St#75",12);
	std::cout << "eng1 : " << eng1 << std::endl;	
    */

	std::cout << "---------------------" << std::endl;
	CivilEngineer civil_eng1("John Travolta",51,"Tiny Dog 42St#89",31,"Road Strength");
	std::cout << "civil_eng1 : " << civil_eng1 << std::endl;
	

    return 0;
}