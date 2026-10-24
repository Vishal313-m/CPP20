/* Protected members */


/* Person.h */
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
protected : 
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






/* Player.h */


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
    Player(std::string_view game_param, std::string_view first_name_param,
            std::string_view last_name_param);
    
private : 
    std::string m_game{"None"};
};

#endif // PLAYER_H








/* Player.cpp*/

#include "person.h"
#include "player.h"

Player::Player(std::string_view game_param, std::string_view first_name_param,
            std::string_view last_name_param)
{
    m_game = game_param;
    first_name = first_name_param;
    last_name = last_name_param;
}

std::ostream& operator<<(std::ostream& out, const Player& player){
 
    out << "Player : [ game : "  << player.m_game
         << " names : " << player.get_first_name()
             << " " << player.get_last_name() << "]";
    return out;
}






/* main.cpp*/


#include <iostream>
#include "player.h"


int main(){

    Player p1("Basketball","John","Snow");
    std::cout << "player : " << p1 << std::endl;
   
    return 0;
}


