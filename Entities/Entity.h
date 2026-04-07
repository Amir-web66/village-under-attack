
#pragma once
#include <string>
#include "../Position.h"
class Entity{
protected:
    Position pos_;
    std::string repr_;
public:
    Entity(const std::string& r,Position p={0,0});
    virtual ~Entity()=default;
    Position    getPosition()const{return pos_;}
    std::string getRepr()    const{return repr_;}
    void setPosition(Position p){pos_=p;}
    virtual std::string getType()const=0;
};
