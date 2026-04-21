#pragma once
#include "Building.h"

class Barrack : public Building {
public:
    static constexpr int SIZE = 3;
    static constexpr int HP = 40;
    static constexpr int COST_GOLD = 100;
    static constexpr int MAX_INST = 1;

    Barrack();
    std::string getType() const override { return "Barrack"; }
};
