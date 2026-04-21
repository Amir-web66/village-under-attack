
#include "Raider.h"
Raider::Raider(Position p,int wave)
    :Enemy("🏴",15+wave*3,3+wave/3,0.8f+wave*0.05f,1.2f,p){}
