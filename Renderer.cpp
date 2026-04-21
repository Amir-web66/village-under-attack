#include "Renderer.h"
#include <cstdio>
#include <cstring>
#include <string>
#include <sstream>
#include <algorithm>

// Codes ANSI
#define RST  "\033[0m"
#define YEL  "\033[33m"
#define BYL  "\033[93m"
#define DIM  "\033[90m"
#define WHT  "\033[97m"
#define GRN  "\033[92m"
#define RED  "\033[91m"
#define CYN  "\033[96m"
#define MGT  "\033[95m"

static const int W = Board::WIDTH;
static const int H = Board::HEIGHT;

// ─── Utilitaires string ───────────────────────────────────────────────────────

// Compte les octets d'un caractere UTF-8 lead byte
static int utf8len(unsigned char c){
    if(c<0x80) return 1;
    if((c&0xE0)==0xC0) return 2;
    if((c&0xF0)==0xE0) return 3;
    if((c&0xF8)==0xF0) return 4;
    return 1;
}

// Largeur visuelle d'une string UTF-8 (emoji = 2, autres = 1)
static int visWidth(const std::string& s){
    int w=0;
    for(int i=0;i<(int)s.size();){
        unsigned char c=(unsigned char)s[i];
        // Ignore les sequences ANSI
        if(c=='\033'){
            while(i<(int)s.size()&&s[i]!='m') i++;
            i++; continue;
        }
        int bl=utf8len(c);
        // 4 octets = emoji Unicode supplementaire -> largeur 2
        w+=(bl==4)?2:1;
        i+=bl;
    }
    return w;
}

// Pad la string a exactement width colonnes visuelles
static std::string padTo(const std::string& s, int width){
    int v=visWidth(s);
    std::string r=s;
    while(v<width){r+=' ';v++;}
    return r;
}

// Ligne sidebar : "│ label        value │"
static std::string sline(const char* lbl, const std::string& val,
                          int sw, const char* valCol=WHT){
    std::string content = std::string(" ")+DIM+lbl+RST;
    // Calcul du nb d'espaces entre label et valeur
    int used = 1 + (int)strlen(lbl) + (int)visWidth(val);
    int spaces = sw - used - 1;  // -1 pour l'espace final
    if(spaces<1) spaces=1;
    content += std::string(spaces,' ');
    content += valCol + val + RST;
    content  = padTo(content, sw);
    return std::string(YEL)+"\xe2\x94\x82"+RST + content + std::string(YEL)+"\xe2\x94\x82"+RST;
}

// Ligne sidebar centrée
static std::string scenter(const std::string& s, int sw, const char* col=WHT){
    int v=visWidth(s);
    if(v>=sw){
        // If content is wider than the sidebar, left-align and let padTo clamp width safely.
        std::string content = std::string(col) + s + RST;
        content = padTo(content, sw);
        return std::string(YEL)+"\xe2\x94\x82"+RST+content+std::string(YEL)+"\xe2\x94\x82"+RST;
    }
    int lpad=(sw-v)/2, rpad=sw-v-lpad;
    std::string content = std::string(lpad,' ')+col+s+RST+std::string(rpad,' ');
    return std::string(YEL)+"\xe2\x94\x82"+RST+content+std::string(YEL)+"\xe2\x94\x82"+RST;
}

// Ligne vide
static std::string sempty(int sw){
    return std::string(YEL)+"\xe2\x94\x82"+RST+std::string(sw,' ')+std::string(YEL)+"\xe2\x94\x82"+RST;
}

// Ligne horizontale ├────┤
static std::string shline(int sw){
    std::string s=std::string(YEL)+"\xe2\x94\x9c";
    for(int i=0;i<sw;i++) s+="\xe2\x94\x80";
    s+="\xe2\x94\xa4"+std::string(RST);
    return s;
}
static std::string sTop(int sw){
    std::string s=std::string(YEL)+"\xe2\x94\x8c";
    for(int i=0;i<sw;i++) s+="\xe2\x94\x80";
    s+="\xe2\x94\x90"+std::string(RST);
    return s;
}
static std::string sBot(int sw){
    std::string s=std::string(YEL)+"\xe2\x94\x94";
    for(int i=0;i<sw;i++) s+="\xe2\x94\x80";
    s+="\xe2\x94\x98"+std::string(RST);
    return s;
}

// Barre HP colorée
std::string Renderer::hpBar(int hp, int maxHp, int len){
    int filled = (maxHp>0) ? hp*len/maxHp : 0;
    const char* col = (hp > maxHp*2/3) ? GRN : (hp > maxHp/3) ? YEL : RED;
    std::string bar = std::string(col);
    for(int i=0;i<len;i++) bar += (i<filled) ? "\xe2\x96\x88" : "\xe2\x96\x91";
    bar += RST;
    return bar;
}

// ─── Rendu principal ─────────────────────────────────────────────────────────
void Renderer::render(const Board& board,
                      int score, int kills, int wave,
                      float raidCD, bool raidEnabled,
                      bool gameOver,
                      const std::string& msg)
{
    const Resources& res = board.getPlayer().getResources();
    TownHall* th = board.getTownHall();
    int thHp  = (th&&th->isAlive()) ? th->getHealth()    : 0;
    int thMax = th ? th->getMaxHealth() : 50;
    const int mapInnerW = W*2;

    // ── Construire le buffer de la carte ─────────────────────────────────────
    std::string buf[H][W];
    board.fillBuffer(buf);

    // ── Assembler toute la frame dans un ostringstream ────────────────────────
    // (1 seul write => pas de clignotement)
    std::ostringstream out;

    // Home + clear-to-end gives one live frame without piling up history.
    out << "\033[H\033[J";

    // Bordure haute du board
    out << YEL << "\xe2\x94\x8c";
    for(int i=0;i<W*2;i++) out << "\xe2\x94\x80";
    out << "\xe2\x94\x90" << RST << "\n";

    // Lignes du board
    for(int y=0;y<H;y++){
        out << YEL << "\xe2\x94\x82" << RST;
        for(int x=0;x<W;x++) out << buf[y][x];
        out << YEL << "\xe2\x94\x82" << RST << "\n";
    }

    // Bordure basse du board
    out << YEL << "\xe2\x94\x94";
    for(int i=0;i<W*2;i++) out << "\xe2\x94\x80";
    out << "\xe2\x94\x98" << RST << "\n";

    auto pTop = [&](){
        std::string s=std::string(YEL)+"\xe2\x94\x8c";
        for(int i=0;i<mapInnerW;i++) s+="\xe2\x94\x80";
        s+="\xe2\x94\x90"+std::string(RST);
        return s;
    };
    auto pBot = [&](){
        std::string s=std::string(YEL)+"\xe2\x94\x94";
        for(int i=0;i<mapInnerW;i++) s+="\xe2\x94\x80";
        s+="\xe2\x94\x98"+std::string(RST);
        return s;
    };
    auto pLine = [&](const std::string& txt){
        std::string content = padTo(txt, mapInnerW);
        return std::string(YEL)+"\xe2\x94\x82"+RST+content+std::string(YEL)+"\xe2\x94\x82"+RST;
    };

    std::string raidTxt;
    if(!raidEnabled) raidTxt = "PAUSE";
    else if(raidCD>0.f) raidTxt = "NEXT:" + std::to_string((int)raidCD) + "s";
    else raidTxt = "WAVE " + std::to_string(wave);

    std::vector<std::string> panel;
    panel.push_back(pTop());
    panel.push_back(pLine(std::string(BYL)+"  VILLAGE UNDER ATTACK"+RST));
    panel.push_back(pLine(" \xf0\x9f\x8f\x86 " + std::to_string(score) +
                          "   \xf0\x9f\xaa\x99 " + std::to_string(res.getGold()) +
                          "   \xf0\x9f\xa7\xaa " + std::to_string(res.getElixir()) +
                          "   \xf0\x9f\x91\xb9 " + std::to_string(board.getRaiderCount()) +
                          "   \xf0\x9f\x9b\xa1 " + std::to_string(board.getTroopCount()) +
                          "   \xe2\x98\xa0 " + std::to_string(kills)));
    panel.push_back(pLine(" \xf0\x9f\x8f\x9b TownHall " + std::to_string(thHp) + "/" + std::to_string(thMax) +
                          "   " + hpBar(thHp, thMax, 18)));
    panel.push_back(pLine(std::string(" Raid: ") + raidTxt + "   [Arrows] Move  [W/G/E/K] Build  [A/B] Train  [C] Collect  [Q] Quit"));
    {
        std::string msgLine = " Msg: " + msg;
        if((int)msgLine.size()>mapInnerW) msgLine.resize(mapInnerW);
        panel.push_back(pLine(msgLine));
    }
    panel.push_back(pBot());

    for(const auto& ln:panel) out << ln << "\n";

    // Game over overlay
    if(gameOver){
        out << "\n  " << RED
            << "=== GAME OVER ! Le TownHall a ete detruit. ===" << RST << "\n"
            << "  " << BYL
            << "Score : " << score << "   Raiders tues : " << kills << RST << "\n"
            << "  " << DIM << "[Q] Quitter" << RST << "\n";
    }

    // Un seul write
    std::string frame = out.str();
    fwrite(frame.c_str(), 1, frame.size(), stdout);
    fflush(stdout);
}
