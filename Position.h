#pragma once
struct Position {
    int x, y;
    Position(int x=0,int y=0):x(x),y(y){}
    bool operator==(const Position& o)const{return x==o.x&&y==o.y;}
};
