
#include "Building.h"
#include <algorithm>
Building::Building(int sx,int sy,const std::string& r,int hp,int gc,int ec,int mi)
    :sizeX_(sx),sizeY_(sy),repr_(r),goldCost_(gc),elixirCost_(ec),
     maxInstances_(mi),pos_(0,0),health_(hp),maxHealth_(hp),alive_(true){}
bool Building::collidesWith(Position p)const{
    int hx=sizeX_/2,hy=sizeY_/2;
    return p.x>=pos_.x-hx&&p.x<=pos_.x+hx&&p.y>=pos_.y-hy&&p.y<=pos_.y+hy;
}
bool Building::collidesWith(const Building& o)const{
    int ax1=pos_.x-sizeX_/2, ax2=pos_.x+sizeX_/2;
    int ay1=pos_.y-sizeY_/2, ay2=pos_.y+sizeY_/2;
    int bx1=o.pos_.x-o.sizeX_/2, bx2=o.pos_.x+o.sizeX_/2;
    int by1=o.pos_.y-o.sizeY_/2, by2=o.pos_.y+o.sizeY_/2;
    return ax1<=bx2&&ax2>=bx1&&ay1<=by2&&ay2>=by1;
}
void Building::takeDamage(int d){
    health_=std::max(0,health_-d);
    if(health_==0)alive_=false;
}
