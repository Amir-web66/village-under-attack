
#include "ResourceGenerator.h"
#include <algorithm>
ResourceGenerator::ResourceGenerator(int sx,int sy,const std::string& r,int hp,int gc,int ec,int mi,float rate,float cap)
    :Building(sx,sy,r,hp,gc,ec,mi),current_(0.f),capacity_(cap),ratePerSec_(rate){}
void ResourceGenerator::update(float dt){
    if(!isFull())current_=std::min(capacity_,current_+ratePerSec_*dt);
}
void ResourceGenerator::collect(int& out){out=(int)current_;current_=0.f;}
