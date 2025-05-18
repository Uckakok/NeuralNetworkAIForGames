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
    /// <summary>
    /// Lists all saved models for the given game.
    /// </summary>
    /// <param name="game">Pointer to the game instance for which to list saves.</param>
    static void ListSaves(IGame* game);
    void Run();

private:
    struct Step {
        std::vector<float> BoardState;
        float ValueEstimate;
        float Reward;
        int Player;
    };


    struct Player
    {
        std::unique_ptr<NeuralNetwork> NN;
        float Wins = 0;
        int Losses = 0;
    };


    int m_championImprovements = 0;
    
    float m_epsilon = 0.2f;
    int m_MCTSEpisodes = 100;
    int m_populationSize = 40;
    int m_matchesPerIteration = 4;
    float m_learningRate = 0.1f;
    std::vector<Player> m_population;
    int m_championId = -1;

    std::unique_ptr<IGame> m_baseGame;
    int m_mutationRate = 1;

    NeuralNetwork* m_loadedNetwork;

    /// <summary>Plays a match using PPO training with a single neural network.</summary>
    IGame::Winner PlayMatchPPO(NeuralNetwork* nn1);
    /// <summary>Measures the range of evaluation values a neural network gives across random games. Used for Monte Carlo to work properly.</summary>
    void FuzzEvaluationExtremes(const IGame& baseGame, NeuralNetwork* network, int nGames, float& outMinEval, float& outMaxEval, bool log = true);
    void ChangeParametersMenu();
    /// <summary>Trains the neural network against a random player for a number of generations using evolutionary algorithm.</summary>
    void TrainIterationsAgainstRandom(int generations);
    /// <summary>Benchmarks current neural network against random moves.</summary>
    void TestChampionAgainstRandom(int games);
    void TrainIterations(int n);
    int ChooseBestMove(const IGame& game, const NeuralNetwork* network);
    void ApplyPPORewards(NeuralNetwork* nn, std::vector<Step>& history);
    IGame::Winner PlayMatch(NeuralNetwork* nn1, NeuralNetwork* nn2);
    void TrainIterationsPPO(int generations);
    void EvaluateAndPromoteChampion();

    std::unique_ptr<IGame> CloneGameWithMove(const IGame& game, int move);
};
