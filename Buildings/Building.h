
#pragma once
#include <string>
#include "../Position.h"

class Building {
protected:
    int sizeX_, sizeY_;
    std::string repr_;
    int goldCost_, elixirCost_, maxInstances_;
    Position pos_;
    int health_, maxHealth_;
    bool alive_;
public:
    Building(int sx,int sy,const std::string& r,int hp,int gc,int ec,int mi);
    virtual ~Building()=default;
    void setPosition(Position p){pos_=p;}
    Position    getPosition()    const{return pos_;}
    int         getSizeX()       const{return sizeX_;}
    int         getSizeY()       const{return sizeY_;}
    int         getHealth()      const{return health_;}
    int         getMaxHealth()   const{return maxHealth_;}
    int         getGoldCost()    const{return goldCost_;}
    int         getElixirCost()  const{return elixirCost_;}
    int         getMaxInstances()const{return maxInstances_;}
    bool        isAlive()        const{return alive_&&health_>0;}
    Position getTopLeft()const{return{pos_.x-sizeX_/2,pos_.y-sizeY_/2};}
    bool collidesWith(Position p)const;
    bool collidesWith(const Building& o)const;
    virtual void takeDamage(int d);
    virtual std::string getRepr()const{return repr_;}
    virtual void update(float dt){(void)dt;}
    virtual std::string getType()const{return "Building";}
};
