#pragma once
#include "Enemy.h"
#include <vector>

class Bomberman : public Enemy {
public:
    explicit Bomberman(Position p={0,0});

    void update(float dt, const std::vector<Building*>& bs, int maxX, int maxY);
    std::string getType() const override { return "Bomberman"; }
};
