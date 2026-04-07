
#include "Enemy.h"
#include <cmath>
#include <limits>
Enemy::Enemy(const std::string& r,int hp,int dmg,float spd,float atkRate,Position p)
    :Npc(r,hp,p),damage_(dmg),speed_(spd),atkRate_(atkRate),atkTimer_(0.f),moveAcc_(0.f),target_(nullptr){}
void Enemy::findTarget(const std::vector<Building*>& bs){
    target_=nullptr; float best=std::numeric_limits<float>::max();
    for(auto* b:bs){
        if(!b||!b->isAlive())continue;
        float dx=(float)(b->getPosition().x-pos_.x);
        float dy=(float)(b->getPosition().y-pos_.y);
        float d=std::sqrt(dx*dx+dy*dy);
        if(d<best){best=d;target_=b;}
    }
}
void Enemy::update(float dt,const std::vector<Building*>& bs){
    if(!isAlive())return;
    if(!target_||!target_->isAlive())findTarget(bs);
    if(!target_)return;
    int dx=target_->getPosition().x-pos_.x;
    int dy=target_->getPosition().y-pos_.y;
    int hx=target_->getSizeX()/2,hy=target_->getSizeY()/2;
    bool adj=std::abs(dx)<=hx+1&&std::abs(dy)<=hy+1;
    if(adj){
        atkTimer_+=dt;
        if(atkTimer_>=atkRate_){target_->takeDamage(damage_);atkTimer_=0.f;}
    } else {
        moveAcc_+=dt;
        float period=1.f/speed_;
        if(moveAcc_>=period){
            moveAcc_=0.f;
            int mx=(dx>0)?1:(dx<0)?-1:0;
            int my=(dy>0)?1:(dy<0)?-1:0;
            if(mx&&my){if(std::abs(dx)>=std::abs(dy))my=0;else mx=0;}
            pos_.x+=mx; pos_.y+=my;
        }
    }
}
