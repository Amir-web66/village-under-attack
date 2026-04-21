#pragma once
#include "Troop.h"

class Archer : public Troop {
public:
    static constexpr int COST_ELIXIR = 50;

    explicit Archer(Position p={0,0});
    std::string getType() const override { return "Archer"; }
};
