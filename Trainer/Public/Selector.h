#pragma once
#include "IGame.h"
#include "NeuralNetwork.h"
#include <vector>
#include <Windows.h>


typedef IGame* (*CreateGameFunc)();
typedef const char* (*GetGameNameFunc)();

class GameSelector {
public:
    void Start();
private:
    struct GameEntry {
        HMODULE Lib;
        std::string Name;
        CreateGameFunc CreateFunc;
    };
    std::vector<GameEntry> m_loadedGames;

    void PlayGameLoop(std::unique_ptr<IGame> game, NeuralNetwork* aiNetwork, int humanPlayer);
    void PlayHotSeat(std::unique_ptr<IGame> game);
    void PlayAgainstAI(std::unique_ptr<IGame> game);
    void LoadGameDLLs();
};
