#pragma once
class Resources {
    int gold_, elixir_;
public:
    Resources(int g=0,int e=0):gold_(g),elixir_(e){}
    int  getGold()   const{return gold_;}
    int  getElixir() const{return elixir_;}
    void addGold(int n)   {gold_  +=n;}
    void addElixir(int n) {elixir_+=n;}
    bool canAfford(int gc,int ec)const{return gold_>=gc&&elixir_>=ec;}
    bool spend(int gc,int ec){
        if(!canAfford(gc,ec))return false;
        gold_-=gc; elixir_-=ec; return true;
    }
};
