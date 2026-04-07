
#pragma once
#include "ResourceGenerator.h"
class ElixirCollector:public ResourceGenerator{
public:
    ElixirCollector();
    std::string getRepr()const override;
    std::string getType()const override{return "ElixirCollector";}
};
