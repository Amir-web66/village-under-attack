#pragma once
#include "Board.h"
#include <string>

// Rendu console SANS clignotement :
//  - On construit toute la frame dans un std::string
//  - Un seul write() final
//  - Le curseur reste en position 0,0 (pas d'effacement total)
class Renderer {
public:
    static const int SW = 27;  // largeur sidebar (contenu)

    static void render(const Board& board,
                       int score, int kills, int wave,
                       float raidCD, bool raidEnabled,
                       bool gameOver,
                       const std::string& msg);
private:
    static std::string hpBar(int hp, int maxHp, int len);
};
