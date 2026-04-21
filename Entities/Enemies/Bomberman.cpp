#include "Bomberman.h"
#include "../../Buildings/Wall.h"
#include <cmath>
#include <limits>

Bomberman::Bomberman(Position p)
    : Enemy("💣", 35, 25, 0.7f, 1.4f, p) {}

void Bomberman::update(float dt, const std::vector<Building*>& bs, int maxX, int maxY) {
    if (!isAlive()) return;

    Building* preferred = nullptr;
    float best = std::numeric_limits<float>::max();

    for (auto* b : bs) {
        if (!b || !b->isAlive()) continue;
        if (!dynamic_cast<Wall*>(b)) continue;

        float dx = static_cast<float>(b->getPosition().x - pos_.x);
        float dy = static_cast<float>(b->getPosition().y - pos_.y);
        float d = std::sqrt(dx * dx + dy * dy);
        if (d < best) {
            best = d;
            preferred = b;
        }
    }

    if (preferred) {
        target_ = preferred;
    } else if (!target_ || !target_->isAlive()) {
        findTarget(bs);
    }

    if (!target_) return;

    int dx = target_->getPosition().x - pos_.x;
    int dy = target_->getPosition().y - pos_.y;
    int hx = target_->getSizeX() / 2;
    int hy = target_->getSizeY() / 2;
    bool adj = std::abs(dx) <= hx + 1 && std::abs(dy) <= hy + 1;

    if (adj) {
        atkTimer_ += dt;
        if (atkTimer_ >= atkRate_) {
            target_->takeDamage(damage_);
            atkTimer_ = 0.f;
        }
        return;
    }

    moveAcc_ += dt;
    float period = 1.f / speed_;
    if (moveAcc_ >= period) {
        moveAcc_ = 0.f;
        int mx = (dx > 0) ? 1 : (dx < 0) ? -1 : 0;
        int my = (dy > 0) ? 1 : (dy < 0) ? -1 : 0;
        if (mx && my) {
            if (std::abs(dx) >= std::abs(dy)) my = 0;
            else mx = 0;
        }

        pos_.x += mx;
        pos_.y += my;

        if (pos_.x < 0) pos_.x = 0;
        if (pos_.x >= maxX) pos_.x = maxX - 1;
        if (pos_.y < 0) pos_.y = 0;
        if (pos_.y >= maxY) pos_.y = maxY - 1;
    }
}
