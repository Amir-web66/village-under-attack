
#include "ElixirCollector.h"
ElixirCollector::ElixirCollector():ResourceGenerator(3,3,"\xf0\x9f\xa7\xaa",30,100,0,3,3.f){}
std::string ElixirCollector::getRepr()const{
    return isFull()?"\xf0\x9f\xab\xa7":"\xf0\x9f\xa7\xaa";
}
