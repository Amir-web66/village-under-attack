#pragma once
#include "Troop.h"

class Barbarian : public Troop {
public:
    static constexpr int COST_ELIXIR = 20;

    explicit Barbarian(Position p={0,0});
    std::string getType() const override { return "Barbarian"; }
};
