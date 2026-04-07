#include "Board.h"
#include "Buildings/Wall.h"
#include "Buildings/GoldMine.h"
#include "Buildings/ElixirCollector.h"
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <algorithm>

Board::Board(){ std::memset(grid_,0,sizeof(grid_)); }

void Board::init(){
    std::memset(grid_,0,sizeof(grid_));
    buildings_.clear(); raiders_.clear();
    wallCount_=goldMineCount_=elixirCount_=0;

    auto th=std::make_unique<TownHall>();
    townHall_=th.get();
    addBuilding(std::move(th),{WIDTH/2, HEIGHT/2});

    player_=std::make_unique<Player>(Position{WIDTH/2+5, HEIGHT/2+2});
}

bool Board::canPlace(int cx,int cy,int sx,int sy) const {
    int tlx=cx-sx/2, tly=cy-sy/2;
    for(int y=tly;y<tly+sy;y++)
        for(int x=tlx;x<tlx+sx;x++){
            if(x<0||x>=WIDTH||y<0||y>=HEIGHT) return false;
            if(grid_[y][x]!=nullptr) return false;
        }
    return true;
}

bool Board::addBuilding(std::unique_ptr<Building> b, Position c){
    if(!canPlace(c.x,c.y,b->getSizeX(),b->getSizeY())) return false;
    b->setPosition(c);
    Building* raw=b.get();
    buildings_.push_back(std::move(b));
    regBuilding(raw);
    return true;
}

void Board::regBuilding(Building* b){
    Position tl=b->getTopLeft();
    for(int y=tl.y;y<tl.y+b->getSizeY();y++)
        for(int x=tl.x;x<tl.x+b->getSizeX();x++)
            if(x>=0&&x<WIDTH&&y>=0&&y<HEIGHT) grid_[y][x]=b;
    const std::string& t=b->getType();
    if(t=="Wall")            wallCount_++;
    else if(t=="GoldMine")   goldMineCount_++;
    else if(t=="ElixirCollector") elixirCount_++;
}

void Board::unregBuilding(Building* b){
    Position tl=b->getTopLeft();
    for(int y=tl.y;y<tl.y+b->getSizeY();y++)
        for(int x=tl.x;x<tl.x+b->getSizeX();x++)
            if(x>=0&&x<WIDTH&&y>=0&&y<HEIGHT&&grid_[y][x]==b) grid_[y][x]=nullptr;
    const std::string& t=b->getType();
    if(t=="Wall")            wallCount_=std::max(0,wallCount_-1);
    else if(t=="GoldMine")   goldMineCount_=std::max(0,goldMineCount_-1);
    else if(t=="ElixirCollector") elixirCount_=std::max(0,elixirCount_-1);
}

void Board::removeDeadBuildings(){
    for(auto& b:buildings_) if(b&&!b->isAlive()) unregBuilding(b.get());
    buildings_.erase(std::remove_if(buildings_.begin(),buildings_.end(),
        [](const auto& b){return !b||!b->isAlive();}),buildings_.end());
}

std::vector<Building*> Board::getAliveBuildings() const {
    std::vector<Building*> v;
    for(const auto& b:buildings_) if(b&&b->isAlive()) v.push_back(b.get());
    return v;
}

void Board::spawnRaider(int wave){
    int edge=std::rand()%4;
    Position p;
    switch(edge){
        case 0: p={std::rand()%WIDTH, 0};          break;
        case 1: p={WIDTH-1, std::rand()%HEIGHT};   break;
        case 2: p={std::rand()%WIDTH, HEIGHT-1};   break;
        default:p={0, std::rand()%HEIGHT};         break;
    }
    raiders_.push_back(std::make_unique<Raider>(p,wave));
}

void Board::removeDeadRaiders(int& kills){
    for(auto& r:raiders_) if(r&&!r->isAlive()) kills++;
    raiders_.erase(std::remove_if(raiders_.begin(),raiders_.end(),
        [](const auto& r){return !r||!r->isAlive();}),raiders_.end());
}

int Board::getRaiderCount() const {
    int n=0;
    for(const auto& r:raiders_) if(r&&r->isAlive()) n++;
    return n;
}

void Board::tryCollect(std::string& msg){
    Position pp=player_->getPosition();
    bool got=false;
    for(auto& b:buildings_){
        if(!b||!b->isAlive()) continue;
        bool near=false;
        for(int dy=-1;dy<=1&&!near;dy++)
            for(int dx=-1;dx<=1&&!near;dx++)
                if(b->collidesWith({pp.x+dx,pp.y+dy})) near=true;
        if(!near) continue;
        ResourceGenerator* rg=dynamic_cast<ResourceGenerator*>(b.get());
        if(rg&&rg->getCurrent()>0){
            int amt=0; rg->collect(amt);
            if(b->getType()=="GoldMine"){
                player_->getResources().addGold(amt);
                msg="+" + std::to_string(amt) + " or recolte!";
            } else {
                player_->getResources().addElixir(amt);
                msg="+" + std::to_string(amt) + " elixir recolte!";
            }
            got=true;
        }
    }
    if(!got) msg="Rien a collecter ici.";
}

void Board::update(float dt){
    for(auto& b:buildings_) if(b&&b->isAlive()) b->update(dt);
    auto alive=getAliveBuildings();
    for(auto& r:raiders_)   if(r&&r->isAlive()) r->update(dt,alive);
}

void Board::fillBuffer(std::string buf[HEIGHT][WIDTH]) const {
    // Init vide
    for(int y=0;y<HEIGHT;y++)
        for(int x=0;x<WIDTH;x++)
            buf[y][x]="  ";

    // Batiments
    for(const auto& b:buildings_){
        if(!b||!b->isAlive()) continue;
        Position tl=b->getTopLeft();
        if(b->getType()=="TownHall"){
            const int sx=b->getSizeX();
            const int sy=b->getSizeY();
            const int cx=tl.x+sx/2;
            const int cy=tl.y+sy/2;

            for(int y=tl.y;y<tl.y+sy;y++){
                for(int x=tl.x;x<tl.x+sx;x++){
                    if(x<0||x>=WIDTH||y<0||y>=HEIGHT) continue;
                    const bool top    = (y==tl.y);
                    const bool bottom = (y==tl.y+sy-1);
                    const bool left   = (x==tl.x);
                    const bool right  = (x==tl.x+sx-1);

                    if(x==cx&&y==cy)      buf[y][x]=b->getRepr();
                    else if(top||bottom)  buf[y][x]="--";
                    else if(left)         buf[y][x]="| ";
                    else if(right)        buf[y][x]=" |";
                    else                  buf[y][x]="  ";
                }
            }
            continue;
        }

        Position c=b->getPosition();
        if(c.x>=0&&c.x<WIDTH&&c.y>=0&&c.y<HEIGHT)
            buf[c.y][c.x]=b->getRepr();
    }
    // Raiders
    for(const auto& r:raiders_){
        if(!r||!r->isAlive()) continue;
        Position rp=r->getPosition();
        if(rp.x>=0&&rp.x<WIDTH&&rp.y>=0&&rp.y<HEIGHT)
            buf[rp.y][rp.x]=r->getRepr();
    }
    // Joueur (par dessus tout)
    Position pp=player_->getPosition();
    if(pp.x>=0&&pp.x<WIDTH&&pp.y>=0&&pp.y<HEIGHT)
        buf[pp.y][pp.x]=player_->getRepr();
}
