
#include "Player.h"
#include <algorithm>
Player::Player(Position p):Entity("\xf0\x9f\xa7\x91",p),resources_(400,400){}
void Player::move(int dx,int dy,int maxX,int maxY){
    pos_.x=std::max(0,std::min(maxX-1,pos_.x+dx));
    pos_.y=std::max(0,std::min(maxY-1,pos_.y+dy));
}
