#include "Troop.h"
#include <algorithm>
#include <cmath>

Troop::Troop(const std::string& r, int hp, int dmg, int range, float speed, float atkRate, Position p)
    : Npc(r, hp, p),
      damage_(dmg),
      range_(range),
      speed_(speed),
      atkRate_(atkRate),
      atkTimer_(0.f),
      moveAcc_(0.f),
      idlePos_(p),
      target_(nullptr),
      state_(TroopState::IDLE) {}

void Troop::setTarget(Enemy* e) {
    if (e && e->isAlive()) {
        target_ = e;
        state_ = TroopState::ENGAGING;
    } else {
        target_ = nullptr;
        state_ = TroopState::IDLE;
    }
}

void Troop::update(float dt, const std::vector<Enemy*>& enemies, int maxX, int maxY) {
    if (!isAlive()) {
        state_ = TroopState::DEAD;
        return;
    }

    if ((!target_ || !target_->isAlive()) && !enemies.empty()) {
        for (Enemy* e : enemies) {
            if (e && e->isAlive()) {
                setTarget(e);
                break;
            }
        }
    }

    if (!target_ || !target_->isAlive()) {
        target_ = nullptr;
        state_ = TroopState::IDLE;

        int dx = idlePos_.x - pos_.x;
        int dy = idlePos_.y - pos_.y;
        if (dx == 0 && dy == 0) return;

        moveAcc_ += dt;
        float period = 1.f / std::max(0.1f, speed_);
        if (moveAcc_ >= period) {
            moveAcc_ = 0.f;
            int mx = (dx > 0) ? 1 : (dx < 0) ? -1 : 0;
            int my = (dy > 0) ? 1 : (dy < 0) ? -1 : 0;
            if (mx && my) {
                if (std::abs(dx) >= std::abs(dy)) my = 0;
                else mx = 0;
            }
            pos_.x = std::clamp(pos_.x + mx, 0, maxX - 1);
            pos_.y = std::clamp(pos_.y + my, 0, maxY - 1);
        }
        return;
    }

    state_ = TroopState::ENGAGING;
    int dx = target_->getPosition().x - pos_.x;
    int dy = target_->getPosition().y - pos_.y;
    int dist = std::max(std::abs(dx), std::abs(dy));

    if (dist <= range_) {
        atkTimer_ += dt;
        if (atkTimer_ >= atkRate_) {
            target_->takeDamage(damage_);
            atkTimer_ = 0.f;
        }
    } else {
        moveAcc_ += dt;
        float period = 1.f / std::max(0.1f, speed_);
        if (moveAcc_ >= period) {
            moveAcc_ = 0.f;
            int mx = (dx > 0) ? 1 : (dx < 0) ? -1 : 0;
            int my = (dy > 0) ? 1 : (dy < 0) ? -1 : 0;
            if (mx && my) {
                if (std::abs(dx) >= std::abs(dy)) my = 0;
                else mx = 0;
            }
            pos_.x = std::clamp(pos_.x + mx, 0, maxX - 1);
            pos_.y = std::clamp(pos_.y + my, 0, maxY - 1);
        }
    }
}
