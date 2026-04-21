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
    buildings_.clear();
    raiders_.clear();
    bombermen_.clear();
    troops_.clear();
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

    // ── Mise à jour des Bombermen ──
    std::vector<Building*> bldgPtrs;
    for (auto& b : buildings_) if (b && b->isAlive()) bldgPtrs.push_back(b.get());
    for (auto& b : bombermen_) if (b && b->isAlive()) b->update(dt, bldgPtrs, WIDTH, HEIGHT);

    // ── Dispatch stratégique : max 2 troupes par ennemi ──
    auto aliveEnemies = getAliveEnemies();
    for (Enemy* enemy : aliveEnemies) {
        int assigned = 0;
        for (auto& t : troops_)
            if (t && t->getState() == TroopState::ENGAGING && t->getTarget() == enemy)
                assigned++;
        for (auto& t : troops_) {
            if (assigned >= 2) break;
            if (t && t->getState() == TroopState::IDLE) {
                t->setTarget(enemy);
                assigned++;
            }
        }
    }

    // ── Mise à jour des troupes ──
    for (auto& t : troops_) if (t && t->isAlive()) t->update(dt, aliveEnemies, WIDTH, HEIGHT);
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

    // Troupes
    for(const auto& t:troops_){
        if(!t||!t->isAlive()) continue;
        Position tp=t->getPosition();
        if(tp.x>=0&&tp.x<WIDTH&&tp.y>=0&&tp.y<HEIGHT)
            buf[tp.y][tp.x]=t->getRepr();
    }

    // Bombermen
    for(const auto& b:bombermen_){
        if(!b||!b->isAlive()) continue;
        Position bp=b->getPosition();
        if(bp.x>=0&&bp.x<WIDTH&&bp.y>=0&&bp.y<HEIGHT)
            buf[bp.y][bp.x]=b->getRepr();
    }

    // Joueur (par dessus tout)
    Position pp=player_->getPosition();
    if(pp.x>=0&&pp.x<WIDTH&&pp.y>=0&&pp.y<HEIGHT)
        buf[pp.y][pp.x]=player_->getRepr();
}
// ── spawnBomberman ──
void Board::spawnBomberman() {
    int side = std::rand() % 4;
    Position spawn;
    switch(side) {
        case 0: spawn = {std::rand() % WIDTH, 0};           break;
        case 1: spawn = {std::rand() % WIDTH, HEIGHT - 1};  break;
        case 2: spawn = {0, std::rand() % HEIGHT};          break;
        default: spawn = {WIDTH - 1, std::rand() % HEIGHT}; break;
    }
    bombermen_.push_back(std::make_unique<Bomberman>(spawn));
}

// ── removeDeadBombermen ──
void Board::removeDeadBombermen(int& kills) {
    auto it = std::remove_if(bombermen_.begin(), bombermen_.end(),
        [&](const auto& b) {
            if (!b->isAlive()) { kills++; return true; }
            return false;
        });
    bombermen_.erase(it, bombermen_.end());
}

// ── addTroop ──
void Board::addTroop(std::unique_ptr<Troop> t) {
    troops_.push_back(std::move(t));
}

// ── removeDeadTroops ──
void Board::removeDeadTroops() {
    troops_.erase(std::remove_if(troops_.begin(), troops_.end(),
        [](const auto& t) { return !t->isAlive(); }), troops_.end());
}

// ── getTroopCount ──
int Board::getTroopCount() const { return (int)troops_.size(); }

// ── getBarrackCount ──
int Board::getBarrackCount() const {
    int count = 0;
    for (auto& b : buildings_)
        if (dynamic_cast<Barrack*>(b.get())) count++;
    return count;
}

// ── isPlayerOnBarrack ──
bool Board::isPlayerOnBarrack() const {
    for (auto& b : buildings_)
        if (dynamic_cast<Barrack*>(b.get()) && b->collidesWith(player_->getPosition()))
            return true;
    return false;
}

// ── getTownHallCenter ──
Position Board::getTownHallCenter() const {
    TownHall* th = getTownHall();
    return th ? th->getPosition() : Position{WIDTH/2, HEIGHT/2};
}

// ── getAliveEnemies ──
std::vector<Enemy*> Board::getAliveEnemies() const {
    std::vector<Enemy*> result;
    for (auto& r : raiders_)    if (r && r->isAlive()) result.push_back(r.get());
    for (auto& b : bombermen_)  if (b && b->isAlive()) result.push_back(b.get());
    return result;
}
