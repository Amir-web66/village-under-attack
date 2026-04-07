
#pragma once
#include "ResourceGenerator.h"
class GoldMine:public ResourceGenerator{
public:
    GoldMine();
    std::string getRepr()const override;
    std::string getType()const override{return "GoldMine";}
};
