
#pragma once
#include "../Npc.h"
#include "../../Buildings/Building.h"
#include <vector>
class Enemy:public Npc{
protected:
    int   damage_;
    float speed_;
    float atkRate_,atkTimer_,moveAcc_;
    Building* target_;
public:
    Enemy(const std::string& r,int hp,int dmg,float spd,float atkRate,Position p={0,0});
    bool isAlive()const{return alive_&&health_>0;}
    void findTarget(const std::vector<Building*>& bs);
    void update(float dt,const std::vector<Building*>& bs);
    std::string getType()const override{return "Enemy";}
};
