#pragma once
#include "../Npc.h"
#include "../Enemies/Enemy.h"
#include <vector>

enum class TroopState {
    IDLE,
    ENGAGING,
    DEAD
};

class Troop : public Npc {
protected:
    int damage_;
    int range_;
    float speed_;
    float atkRate_;
    float atkTimer_;
    float moveAcc_;
    Position idlePos_;
    Enemy* target_;
    TroopState state_;

public:
    Troop(const std::string& r, int hp, int dmg, int range, float speed, float atkRate, Position p={0,0});

    TroopState getState() const { return state_; }
    Enemy* getTarget() const { return target_; }
    void setTarget(Enemy* e);
    void setIdlePos(Position p) { idlePos_ = p; }

    virtual void update(float dt, const std::vector<Enemy*>& enemies, int maxX, int maxY);
    std::string getType() const override { return "Troop"; }
};
