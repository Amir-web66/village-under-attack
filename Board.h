#pragma once
#include "Position.h"
#include "Resources.h"
#include "Buildings/Building.h"
#include "Buildings/TownHall.h"
#include "Buildings/ResourceGenerator.h"
#include "Entities/Player.h"
#include "Entities/Enemies/Raider.h"
#include <vector>
#include <memory>
#include <string>

class Board {
public:
    static const int WIDTH  = 30;
    static const int HEIGHT = 20;

private:
    Building* grid_[HEIGHT][WIDTH];
    std::vector<std::unique_ptr<Building>> buildings_;
    std::unique_ptr<Player>               player_;
    std::vector<std::unique_ptr<Raider>>  raiders_;
    TownHall* townHall_ = nullptr;
    int wallCount_=0, goldMineCount_=0, elixirCount_=0;

public:
    Board();
    void init();

    bool canPlace(int cx,int cy,int sx,int sy) const;
    bool addBuilding(std::unique_ptr<Building> b, Position center);
    void removeDeadBuildings();

    Player&       getPlayer()       { return *player_; }
    const Player& getPlayer() const { return *player_; }
    TownHall* getTownHall() const { return townHall_; }
    std::vector<Building*> getAliveBuildings() const;

    void spawnRaider(int wave);
    void removeDeadRaiders(int& kills);
    int  getRaiderCount() const;
    void tryCollect(std::string& msg);
    void update(float dt);

    int getWallCount()     const { return wallCount_;     }
    int getGoldMineCount() const { return goldMineCount_; }
    int getElixirCount()   const { return elixirCount_;   }

    // Rendu : remplit buf (HEIGHT lignes de WIDTH cellules)
    // Chaque cellule est une chaine : emoji (2 cols) ou "  " (2 espaces)
    void fillBuffer(std::string buf[HEIGHT][WIDTH]) const;

private:
    void regBuilding(Building* b);
    void unregBuilding(Building* b);
};
