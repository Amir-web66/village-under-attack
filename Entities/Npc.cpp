
#include "Npc.h"
#include <algorithm>
Npc::Npc(const std::string& r,int hp,Position p):Entity(r,p),health_(hp),maxHealth_(hp),alive_(true){}
void Npc::takeDamage(int d){health_=std::max(0,health_-d);if(health_==0)alive_=false;}
