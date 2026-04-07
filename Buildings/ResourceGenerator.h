
#pragma once
#include "Building.h"
class ResourceGenerator:public Building{
protected:
    float current_,capacity_,ratePerSec_;
public:
    ResourceGenerator(int sx,int sy,const std::string& r,int hp,int gc,int ec,int mi,float rate,float cap=100.f);
    float getCurrent() const{return current_;}
    float getCapacity()const{return capacity_;}
    bool  isFull()     const{return current_>=capacity_;}
    void  collect(int& out);
    void  update(float dt)override;
    std::string getType()const override{return "ResourceGenerator";}
};
