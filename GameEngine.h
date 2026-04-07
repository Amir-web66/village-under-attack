#pragma once
#include "Board.h"
#include "Renderer.h"
#include <string>

class GameEngine {
    Board  board_;
    bool   running_     = true;
    bool   gameOver_    = false;
    bool   raidEnabled_ = false;  // raids bloqués jusqu'au [ESPACE]
    int    score_       = 0;
    int    kills_       = 0;
    int    wave_        = 0;
    float  raidCD_      = 15.f;   // countdown apres le 1er [ESPACE]
    float  raidInterval_= 10.f;
    std::string lastMsg_;

public:
    GameEngine() = default;
    void run();

private:
    void init();
    void processInput(int key);
    void update(float dt);
    void render() const;
};
