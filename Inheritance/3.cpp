   /*  BaseClassAccessSpecifiersADemo */

/* What was public in the base class is going to be public in the derived class. What was protected is going to stay protected.
 What was private is going to become private in the derived class.
In other words, public inheritance is really not going to change the accessibility of the member variables we have in the base class.*/





/* we can also do protected inheritance. And what this is going to do is to make protected the highest access level we can have in the derived class.
So anything that was public in the base class is going to become protected.
Anything that was protected is going to stay protected because that's the highest level we can do now.
And anything that was private in the base class is going to be private.

In the derived class. We say private in the sense that it is going to be private to the base class.
So you can't really use M3 from the player class in this case.If you try to do that, you're going to get a compiler error.*/





/*  We can also do private inheritance and the syntax to do that is what you see here and now private is
  going to be the highest level we can do in the player class in terms of the members that we get from the base class.

So anything that was public is going to become private.
Anything that was protected is going to become private.
Anything that was private to the base class is going to stay private to the base class.This is what we mean here.

But now M1 and M2 just became public to the player class.
If anybody else tries to derive or inherit from a player, these things are not going to be accessible.
And this is in some way the player class being selfish.
It is saying, I am going to get everything I can from my parent class or my base class, but I am going to make it all mine.
Nobody else can take this and this is the kind of setup we can achieve with private inheritance.

 */






/*  Person.h  */
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







/* Person.cpp */

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
 







 /* Player.h */


 #ifndef PLAYER_H
#define PLAYER_H

#include "person.h"

//Player will do public inheritance from Person
class Player : public Person
{
    friend std::ostream& operator<<(std::ostream& out, const Player& player);
public:
    Player();
    ~Player();
    
    //See the access we have to inherited members from Person
    void play(){

        m_full_name = "John Snow"; // OK
        m_age = 55; // OK
        //m_address = "DSAKFD;ASKFJ;DKAS"; Compiler error


    }
private : 
    int m_career_start_year{0};
    double m_salary{0.0};
    int health_factor{0};// Let's say that it's 0~10

};

#endif // PLAYER_H 







/* Player.cpp */ 



#include "person.h"
#include "player.h"
#include <iostream>

Player::Player()
{
}

std::ostream& operator<<(std::ostream& out, const Player& player){
    out << "Player[Full name : " << player.get_full_name() <<
                    ",age : " << player.get_age() << 
                    ",address : " << player.get_address() << "]";
    return out;
}


Player::~Player()
{
}






/* Nurse.h */


#ifndef NURSE_H
#define NURSE_H

#include "person.h"

//Nurse will do protected inheritance
class Nurse : protected Person
{
	friend std::ostream& operator<<(std::ostream& , const Nurse& operand);
public:
	Nurse();
	~Nurse();
	
    void treat_unwell_person(){
        m_full_name = "John Snow"; // OK
        m_age = 23; // OK
        //m_address = "897-78-723"; Compiler error
    }
    
private : 
    int practice_certificate_id{0};
};

#endif // NURSE_H 









/* Nurse .cpp  */ 
#include "person.h"
#include "nurse.h"
#include <iostream>
Nurse::Nurse()
{
}

std::ostream& operator<<(std::ostream& out , const Nurse& operand){
    out << "Nurse [Full name : " << operand.get_full_name() <<
                    ",age : " << operand.get_age() << 
                    ",address : " << operand.get_address() <<
                    ",practice certificate id : " << operand.practice_certificate_id << "]";
    return out;
}


Nurse::~Nurse()
{
}







/* Engineer.h */

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







/* Engineer.cpp */


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









/*main.cpp */


#include <iostream>
#include "person.h"
#include "player.h"
#include "nurse.h"
#include "engineer.h"

int main(){

    Person person1("Daniel Gray",27,"Blue Sky St 233 #56");
    std::cout << "person1 : " << person1 << std::endl;

    std::cout << "---------------" << std::endl;

    Player player;
    player.m_full_name = "Samuel Jackson";
    //player.m_age = 55; Compier error
    //player.m_address = "2i892317322"; Compiler

    std::cout << "------------------" << std::endl;

    Nurse nurse1;
    //nurse1.m_full_name = "Davy Johnes";Compiler error
    //nurse1.m_age = 51; // Compiler error

    std::cout << "---------------" << std::endl;
    Engineer engineer1;
    //engineer1.m_full_name = "Olivier Godson"; // Compiler error
    //engineer1.m_age = 55; // Compiler error
    //engineer1.m_address = "dsakfd;aslfjd;laskf"; // Compiler error
   
    return 0;
}