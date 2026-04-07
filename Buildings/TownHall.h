
#pragma once
#include "Building.h"
class TownHall:public Building{
public:
    TownHall();
    std::string getType()const override{return "TownHall";}
};
