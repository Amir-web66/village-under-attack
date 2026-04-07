
#pragma once
#include "Building.h"
class Wall:public Building{
public:
    Wall();
    std::string getType()const override{return "Wall";}
};
