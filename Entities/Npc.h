
#pragma once
#include "Entity.h"
class Npc:public Entity{
protected:
    int health_,maxHealth_;
    bool alive_;
public:
    Npc(const std::string& r,int hp,Position p={0,0});
    int  getHealth()   const{return health_;}
    bool isAlive()     const{return alive_&&health_>0;}
    virtual void takeDamage(int d);
    std::string getType()const override{return "Npc";}
};
