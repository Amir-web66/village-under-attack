#include "GameEngine.h"
#include "Buildings/Wall.h"
#include "Buildings/GoldMine.h"
#include "Buildings/ElixirCollector.h"
#include "Buildings/Barrack.h"
#include "Entities/Troops/Archer.h"
#include "Entities/Troops/Barbarian.h"
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <thread>
#include <chrono>

// ─── Entree clavier non-bloquante ────────────────────────────────────────────
#ifdef _WIN32
#  include <conio.h>
#  include <windows.h>

static void enableRawMode(){
    // UTF-8 + masquer curseur
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    HANDLE hOut=GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode=0;
    GetConsoleMode(hOut,&mode);
    SetConsoleMode(hOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    // Masquer curseur
    CONSOLE_CURSOR_INFO ci={1,FALSE};
    SetConsoleCursorInfo(hOut,&ci);
    // Effacement initial
    system("cls");
    printf("\033[?25l");
}
static void disableRawMode(){
    HANDLE hOut=GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO ci={1,TRUE};
    SetConsoleCursorInfo(hOut,&ci);
    printf("\033[?25h\033[2J\033[H");
}
static int readKey(){
    if(!_kbhit()) return 0;
    int c=_getch();
    if(c==0||c==224){
        int c2=_getch();
        switch(c2){
            case 72: return 1000; // UP
            case 80: return 1001; // DOWN
            case 77: return 1002; // RIGHT
            case 75: return 1003; // LEFT
        }
        return 0;
    }
    return c;
}

#else
#  include <termios.h>
#  include <unistd.h>
#  include <fcntl.h>

static struct termios s_orig;

static void disableRawMode(){
    tcsetattr(STDIN_FILENO,TCSAFLUSH,&s_orig);
    printf("\033[?25h\033[2J\033[H");
}
static void enableRawMode(){
    tcgetattr(STDIN_FILENO,&s_orig);
    std::atexit(disableRawMode);
    struct termios raw=s_orig;
    raw.c_lflag &= ~(ECHO|ICANON);
    raw.c_cc[VMIN]=0;
    raw.c_cc[VTIME]=0;
    tcsetattr(STDIN_FILENO,TCSAFLUSH,&raw);
    // Passer stdin en non-bloquant
    int flags=fcntl(STDIN_FILENO,F_GETFL,0);
    fcntl(STDIN_FILENO,F_SETFL,flags|O_NONBLOCK);
    // Effacement initial + masquer curseur
    printf("\033[2J\033[H\033[?25l");
    fflush(stdout);
}
static int readKey(){
    char c=0;
    if(read(STDIN_FILENO,&c,1)!=1) return 0;
    if(c=='\033'){
        char seq[3]={0,0,0};
        { ssize_t _r=read(STDIN_FILENO,&seq[0],1); (void)_r; }
        { ssize_t _r=read(STDIN_FILENO,&seq[1],1); (void)_r; }
        if(seq[0]=='['){
            switch(seq[1]){
                case 'A': return 1000;
                case 'B': return 1001;
                case 'C': return 1002;
                case 'D': return 1003;
            }
        }
        return '\033';
    }
    return (unsigned char)c;
}
#endif

// ─────────────────────────────────────────────────────────────────────────────

void GameEngine::init(){
    std::srand((unsigned)std::time(nullptr));
    board_.init();
    raidEnabled_  = false;
    raidCD_       = 15.f;
    raidInterval_ = 10.f;
    wave_=score_=kills_=0;
    lastMsg_ = "Construis des murs et appuie sur [ESPACE] pour lancer les raids!";
}

void GameEngine::run(){
    init();
    enableRawMode();

    using Clock = std::chrono::steady_clock;
    const auto FRAME = std::chrono::milliseconds(100); // 10 FPS
    auto last = Clock::now();

    while(running_){
        auto now = Clock::now();
        float dt = std::chrono::duration<float>(now-last).count();
        last = now;

        int key = readKey();
        processInput(key);

        if(!gameOver_) update(dt);

        render();

        auto elapsed = Clock::now()-now;
        if(elapsed<FRAME)
            std::this_thread::sleep_for(FRAME-elapsed);
    }
    disableRawMode();
}

void GameEngine::processInput(int key){
    if(key=='q'||key=='Q'){ running_=false; return; }
    if(gameOver_) return;

    Player&    p  =board_.getPlayer();
    Resources& res=p.getResources();

    switch(key){
        case 1000: p.move(0,-1,Board::WIDTH,Board::HEIGHT); break;
        case 1001: p.move(0, 1,Board::WIDTH,Board::HEIGHT); break;
        case 1002: p.move(1, 0,Board::WIDTH,Board::HEIGHT); break;
        case 1003: p.move(-1,0,Board::WIDTH,Board::HEIGHT); break;

        case ' ':  // ESPACE = autoriser / lancer les raids
            if(!raidEnabled_){
                raidEnabled_=true;
                lastMsg_="Raids autorises ! Premier raid dans 15 secondes...";
            }
            break;

        case 'w': case 'W':
            if(!res.canAfford(10,0))       { lastMsg_="Pas assez d'or ! (10 requis)"; break; }
            if(board_.getWallCount()>=200)  { lastMsg_="Maximum 200 murs !";           break; }
            if(board_.addBuilding(std::make_unique<Wall>(), p.getPosition())){
                res.spend(10,0);
                lastMsg_="Mur construit ! (-10 or)";
            } else { lastMsg_="Emplacement occupe !"; }
            break;

        case 'g': case 'G':
            if(board_.getGoldMineCount()>=3)  { lastMsg_="Maximum 3 mines d'or !";          break; }
            if(!res.canAfford(0,100))          { lastMsg_="Pas assez d'elixir ! (100 req.)"; break; }
            if(board_.addBuilding(std::make_unique<GoldMine>(), p.getPosition())){
                res.spend(0,100);
                lastMsg_="Mine d'or construite ! (-100 elixir)";
            } else { lastMsg_="Emplacement occupe !"; }
            break;

        case 'e': case 'E':
            if(board_.getElixirCount()>=3) { lastMsg_="Maximum 3 collecteurs !";      break; }
            if(!res.canAfford(100,0))       { lastMsg_="Pas assez d'or ! (100 req.)"; break; }
            if(board_.addBuilding(std::make_unique<ElixirCollector>(), p.getPosition())){
                res.spend(100,0);
                lastMsg_="Collecteur construit ! (-100 or)";
            } else { lastMsg_="Emplacement occupe !"; }
            break;

        case 'c': case 'C':
            board_.tryCollect(lastMsg_);
            break;

        case 'k': case 'K':
    if (board_.getBarrackCount() >= 1) { lastMsg_ = "Maximum 1 caserne !"; break; }
    if (!res.canAfford(Barrack::COST_GOLD, 0)) { lastMsg_ = "Pas assez d'or ! (100 req.)"; break; }
    if (board_.addBuilding(std::make_unique<Barrack>(), p.getPosition())) {
        res.spend(Barrack::COST_GOLD, 0);
        lastMsg_ = "Caserne construite ! (-100 or) | [A]=Archer(50🧪) [B]=Barbarian(20🧪)";
    } else { lastMsg_ = "Emplacement occupe !"; }
    break;

case 'a': case 'A':
    if (!board_.isPlayerOnBarrack()) { lastMsg_ = "Tu dois etre sur la Caserne !"; break; }
    if (!res.canAfford(0, Archer::COST_ELIXIR)) { lastMsg_ = "Pas assez d'elixir ! (50 req.)"; break; }
    {
        Position spawnPos = board_.getTownHallCenter();
        auto archer = std::make_unique<Archer>(spawnPos);
        archer->setIdlePos(spawnPos);
        board_.addTroop(std::move(archer));
        res.spend(0, Archer::COST_ELIXIR);
        lastMsg_ = "Archer entraine ! (-50 elixir) Il defend le centre.";
    }
    break;

case 'b': case 'B':
    if (!board_.isPlayerOnBarrack()) { lastMsg_ = "Tu dois etre sur la Caserne !"; break; }
    if (!res.canAfford(0, Barbarian::COST_ELIXIR)) { lastMsg_ = "Pas assez d'elixir ! (20 req.)"; break; }
    {
        Position spawnPos = board_.getTownHallCenter();
        auto barb = std::make_unique<Barbarian>(spawnPos);
        barb->setIdlePos(spawnPos);
        board_.addTroop(std::move(barb));
        res.spend(0, Barbarian::COST_ELIXIR);
        lastMsg_ = "Barbarian entraine ! (-20 elixir) Il defend le centre.";
    }
    break;
        default: break;
    }
}

void GameEngine::update(float dt){
    board_.update(dt);
    board_.removeDeadBuildings();

    int newKills=0;
    board_.removeDeadRaiders(newKills);
    kills_  += newKills;
    score_  += newKills*15;

    int bombKills = 0;
    board_.removeDeadBombermen(bombKills);
    kills_ += bombKills;
    score_ += bombKills*20;

    board_.removeDeadTroops();
    score_  += (int)(dt*2);  // points passifs

    TownHall* th=board_.getTownHall();
    if(!th||!th->isAlive()){ gameOver_=true; return; }

    if(!raidEnabled_) return;

    raidCD_ -= dt;
    if(raidCD_<=0.f){
        wave_++;
        int count=1+wave_/2;
        for(int i=0;i<count;i++) board_.spawnRaider(wave_);
        if(wave_>=4) board_.spawnBomberman();
        raidInterval_ = std::max(4.f, 10.f - wave_*0.4f);
        raidCD_       = raidInterval_;
        lastMsg_ = ">>> VAGUE " + std::to_string(wave_)
                 + " ! " + std::to_string(count) + " raider(s)"
                 + (wave_ >= 4 ? " + 1 Bomberman" : "") + " <<<";
    }
}

void GameEngine::render() const {
    Renderer::render(board_, score_, kills_, wave_,
                     std::max(0.f,raidCD_), raidEnabled_,
                     gameOver_, lastMsg_);
}
