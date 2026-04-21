CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

SRCS = main.cpp GameEngine.cpp Board.cpp Renderer.cpp \
       Buildings/Building.cpp Buildings/TownHall.cpp Buildings/Wall.cpp \
       Buildings/Barrack.cpp \
       Buildings/ResourceGenerator.cpp Buildings/GoldMine.cpp Buildings/ElixirCollector.cpp \
       Entities/Entity.cpp Entities/Npc.cpp Entities/Player.cpp \
       Entities/Enemies/Enemy.cpp Entities/Enemies/Raider.cpp Entities/Enemies/Bomberman.cpp \
       Entities/Troops/Troop.cpp Entities/Troops/Archer.cpp Entities/Troops/Barbarian.cpp

TARGET = village

all: $(TARGET)
$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^
clean:
	rm -f $(TARGET) village.exe
.PHONY: all clean
