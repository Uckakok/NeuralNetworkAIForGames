#pragma once
#include <vector>
#include <memory>
#include <string>
#include "NeuralNetwork.h"
#include "IGame.h"

class Trainer {
public:
    Trainer(std::unique_ptr<IGame> baseGame);

    NeuralNetwork* GetChampion();
    static void ListSaves(IGame* game);
    void Run();

private:
    struct Step 
    {
        std::vector<float> BoardState;
        float TargetValue;
    };


    struct Player
    {
        std::unique_ptr<NeuralNetwork> NN;
        float Wins = 0;
        int Losses = 0;
    };


    int m_championImprovements = 0;

    int m_populationSize = 40;
    int m_matchesPerIteration = 4;
    float m_learningRate = 0.1f;
    std::vector<Player> m_population;
    int m_championId = -1;

    std::unique_ptr<IGame> m_baseGame;
    int m_mutationRate = 1;

    NeuralNetwork* m_loadedNetwork;

    void FuzzEvaluationExtremes(const IGame& baseGame, NeuralNetwork* network, int nGames, float& outMinEval, float& outMaxEval);
    void ChangeParametersMenu();
    void TrainIterationsAgainstRandom(int generations);
    void TestChampionAgainstRandom(int games);
    void TrainIterations(int n);
    int ChooseBestMove(const IGame& game, const NeuralNetwork* network);
    IGame::Winner PlayMatch(NeuralNetwork* nn1, NeuralNetwork* nn2);
    IGame::Winner PlayMatchGD(NeuralNetwork* nn1, NeuralNetwork* nn2);
    void ApplyRewards(NeuralNetwork* nn, std::vector<Step>& history, float finalReward);
    void TrainIterationsGD(int generations);
    void EvaluateAndPromoteChampion();

    std::unique_ptr<IGame> CloneGameWithMove(const IGame& game, int move);
};
