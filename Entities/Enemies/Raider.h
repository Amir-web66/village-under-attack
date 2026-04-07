
#pragma once
#include "Enemy.h"
class Raider:public Enemy{
public:
    explicit Raider(Position p,int wave=1);
    std::string getType()const override{return "Raider";}
};
