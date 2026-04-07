
#pragma once
#include "Entity.h"
#include "../Resources.h"
class Player:public Entity{
    Resources resources_;
public:
    explicit Player(Position p={0,0});
    Resources&       getResources()      {return resources_;}
    const Resources& getResources()const {return resources_;}
    void move(int dx,int dy,int maxX,int maxY);
    std::string getType()const override{return "Player";}
};
