/*  With Public Inheritance , derived class can access and use public members of the base class but derived class
can't directly access private members      */

/* The same also applies to friends of the derived class , They have access to private members of derived but don't have 
access to the base class */


Hopefully this proves to you that any derived class object is going to have a base part embedded into
it, and that's going to be the foundation on top of which the derived class is going to be building its own.

/* 

Public Inheritance - When members of the base class are derived publicly, then the protected members of the base class
 will become protected members in the derived class and public members of the base class will become public members in the derived class.


Private Inheritance  ( By default ) - If the derived class inherits the properties of the base class in private mode,
 then both public and protected members will become private in the derived class. 


Protected mode - If the derived class inherits the properties of the base class in protected mode, 
then both public and protected members will become protected in the derived class.

*/




/* Person.h*/
#ifndef PERSON_H
#define PERSON_H

#include <string>
#include <iostream>

class Person
{
    friend std::ostream& operator<<(std::ostream& out, const Person& person);
public:
    Person();
    Person(std::string& first_name_param, std::string& last_name_param);
    ~Person();
    
    //Getters
    std::string get_first_name() const{
        return first_name;
    }
    
    std::string get_last_name() const{
        return last_name;
    }
	
    //Setters
    void set_first_name(std::string_view fn){
        first_name = fn;
    }
    void set_last_name(std::string_view ln){
        last_name = ln;
    }
private : 
    std::string first_name{"Mysterious"};
    std::string last_name{"Person"};
};


#endif // PERSON_H




/* Person.cpp*/

#include "person.h"

Person::Person(){
}

Person::Person(std::string& first_name_param, std::string& last_name_param)
    : first_name(first_name_param), last_name(last_name_param)
{
}

std::ostream& operator<<(std::ostream& out , const Person& person){
    out << "Person [" << person.first_name << " " << person.last_name << "]";
    return out;
}


Person::~Person()
{

}







/*  Player .h */

#ifndef PLAYER_H
#define PLAYER_H

#include <string>
#include <iostream>
#include <string_view>
#include "person.h"

class Player : public Person
{
    friend std::ostream& operator<<(std::ostream& out, const Player& player);
public:
    Player() = default;
    Player(std::string_view game_param);
    
private : 
    std::string m_game{"None"};
};

#endif // PLAYER_H





/*  player .cpp*/
#include "person.h"
#include "player.h"

Player::Player(std::string_view game_param)
    : m_game(game_param)
{
    //first_name = "John"; Compiler errors
    //last_name = "Snow";
}

std::ostream& operator<<(std::ostream& out, const Player& player){
 
    out << "Player : [ game : "  << player.m_game
         << " names : " << player.get_first_name()
             << " " << player.get_last_name() << "]";
    return out;
}







/* Main.cpp*/

include <iostream>
#include "player.h"


int main(){

    Player p1("Basketball");   



    /*   Another thing I want you to see is that we can actually really prove that any player object is going
    to have a person part of it baked in, and that's going to be storing the information that is modeled into our person class.

   And we can use a debugger to do that.

For this to work, I am going to put a breakpoint here on line eight, which is printing things out.
This breakpoint is going to be hit after our object is set up and we can really see the data inside this guy here.

So what we're going to do, we're going to hit this run tab and we're going to use G plus plus.
We're going to use the debugging tools that come with GCC because this is our favorite.
We're going to hit the green button here.
This is going to start our debugging session.
A binary is going to be built and it is going to be plugged into our debugger and we're going to hit our breakpoint here.

Now, if you look on the left here, you're going to see a title that says locals, We're going to expand
this and we're going to see that the P one object actually has a person part of it.
And you see that Visual Studio code is being helpful.

Here it is saying that this is the base class and if we expand, we're going to see that we have our
member variables, we have first name and last name, and this is a player object that we are looking at.

Don't forget that.
And on top of our person part, we also have our own member variable, which is the game that we play and it is right here.

Hopefully this proves to you that any derived class object is going to have a base part embedded into
it, and that's going to be the foundation on top of which the derived class is going to be building its own.
*/
    p1.set_first_name("John");
    p1.set_last_name("Snow");
    std::cout << "player : " << p1 << std::endl;
   
    return 0;
}