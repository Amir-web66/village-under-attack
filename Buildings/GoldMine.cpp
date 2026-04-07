
#include "GoldMine.h"
GoldMine::GoldMine():ResourceGenerator(3,3,"\xe2\x9b\x8f",30,0,100,3,5.f){}
std::string GoldMine::getRepr()const{
    return isFull()?"\xf0\x9f\xaa\x99":"\xe2\x9b\x8f";
}
