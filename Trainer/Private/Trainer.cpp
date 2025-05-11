#include "Trainer.h"
#include <iostream>
#include <limits>
#include <future>
#include <random>
#include <limits>
#include "MonteCarlo.h"
#define NOMINMAX
#include <windows.h>
#include <numeric>
#include <algorithm>
#include "Selector.h"
#include <unordered_set>

Trainer::Trainer(std::unique_ptr<IGame> baseGame)
    : m_baseGame(std::move(baseGame)), m_championImprovements(0)
{
    ;
}



void Trainer::Run() 
{
    std::cout << "=== Neural Network Trainer Setup ===\n";

    int numLayers = 0;
    std::vector<int> layerSizes;

    std::cout << "Enter number of hidden layers: ";
    std::cin >> numLayers;

    for (int i = 0; i < numLayers; ++i) 
    {
        int neurons = 0;
        std::cout << "Enter number of neurons in hidden layer " << (i + 1) << ": ";
        std::cin >> neurons;
        layerSizes.push_back(neurons);
    }

    m_population.clear();
    for (int i = 0; i < m_populationSize; ++i) 
    {
        m_population.emplace_back(Player{
            std::make_unique<NeuralNetwork>(
                m_baseGame->GetBoardState().size(),
                layerSizes
            ), 0
            });
    }

    while (true) 
    {
        std::cout << "\n=== Neural Network Trainer ===\n";
        std::cout << "1. Play against AI\n";
        std::cout << "2. Train N iterations\n";
        std::cout << "3. Save current best\n";
        std::cout << "4. Test champion vs random player (1000 games)\n";
        std::cout << "5. Train N iterations against random\n";
        std::cout << "6. Change parameters\n";
        std::cout << "7. Train N iterations with gradient descent\n";
        std::cout << "8. Load Neural network\n";
        std::cout << "9. Fuzz extremes\n";
        std::cout << "0. Exit\n";
        std::cout << "Choice: ";

        int choice;
        std::cin >> choice;

        if (std::cin.fail()) 
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }

        switch (choice)
        {
        case 1:
        {
            if (GetChampion() == nullptr)
            {
                std::cout << "No champion to play against.\n";
                break;
            }
            if (!GetChampion()->ClampedEvaluationPossible())
            {
                std::cout << "Bounds aren't set for this neural network!\n";
                break;
            }

            int userPlayer = 0;
            while (userPlayer != 1 && userPlayer != 2)
            {
                std::cout << "Play as Player 1 or 2? ";
                std::cin >> userPlayer;

                if (std::cin.fail() || (userPlayer != 1 && userPlayer != 2))
                {
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    std::cout << "Invalid input. Enter 1 or 2.\n";
                }
            }

            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            GameSelector::PlayGameLoop(std::move(m_baseGame), GetChampion(), userPlayer);
            break;
        }
        case 2: {
            std::cout << "Enter number of iterations: ";
            int iterations;
            std::cin >> iterations;
            if (!std::cin.fail() && iterations > 0) 
            {
                TrainIterations(iterations);
            }
            else 
            {
                std::cout << "Invalid number.\n";
            }
            break;
        }
        case 3: 
        {
            NeuralNetwork* ai = GetChampion();
            if (ai == nullptr)
            {
                std::cout << "No champion to save!\n";
                break;
            }
            ai->Save(m_baseGame->GetName());
            std::cout << "Saved current champion as " << m_baseGame->GetName() << ai->Id << ".nn" << "\n";
            break;
        }
        case 4:
            TestChampionAgainstRandom(1000);
            break;
        case 5:
            std::cout << "Train against random agent only. Enter iterations: ";
            int randGens;
            std::cin >> randGens;
            if (!std::cin.fail() && randGens > 0) 
            {
                TrainIterationsAgainstRandom(randGens);
            }
            break;
        case 6:
            ChangeParametersMenu();
            break;
        case 7: {
            std::cout << "Enter number of iterations: ";
            int gdIters;
            std::cin >> gdIters;
            if (!std::cin.fail() && gdIters > 0)
            {
                TrainIterationsGD(gdIters);
            }
            else
            {
                std::cout << "Invalid number.\n";
            }
            break;
        }
        case 8:
        {
            ListSaves(m_baseGame.get());
            std::cout << "Enter name of save to load: ";
            std::string name;
            std::cin >> name;
            if (!std::cin.fail()) 
            {
                try
                {
                    NeuralNetwork loaded = NeuralNetwork::Load(name);
                    m_population.clear();
                    for (int i = 0; i < m_populationSize; ++i) 
                    {
                        m_population.emplace_back(Player{
                            std::make_unique<NeuralNetwork>(loaded), 0
                            });
                    }
                    m_championId = m_population[0].NN.get()->Id;
                    std::cout << "Loaded network and updated population.\n";
                }
                catch (...) 
                {
                    std::cout << "Failed to load network: " << name << "\n";
                }
            }
            else 
            {
                std::cout << "Failed to read name.\n";
            }
            break;
        }
        case 9: 
        {
            std::cout << "Enter number of random games to fuzz: ";
            int n;
            std::cin >> n;
            if (std::cin.fail() || n <= 0) 
            {
                std::cout << "Invalid number.\n";
                break;
            }

            NeuralNetwork* nn = GetChampion();
            if (nn == nullptr) 
            {
                std::cout << "No champion available.\n";
                break;
            }

            float minEval, maxEval;
            FuzzEvaluationExtremes(*m_baseGame, nn, n, minEval, maxEval);
            nn->SetKnownEvaluationBounds(minEval, maxEval);
            std::cout << "Clamping range updated to [" << minEval << ", " << maxEval << "]\n";
            break;
        }
        case 0:
            return;
        default:
            std::cout << "Invalid choice.\n";
        }
    }
}


void Trainer::FuzzEvaluationExtremes(const IGame& baseGame, NeuralNetwork* network, int nGames, float& outMinEval, float& outMaxEval)
{
    std::random_device rd;
    std::mt19937 gen(rd());

    outMinEval = std::numeric_limits<float>::max();
    outMaxEval = std::numeric_limits<float>::lowest();

    for (int i = 0; i < nGames; ++i) 
    {
        auto game = baseGame.Clone();

        while (game->GetWinner() == IGame::Winner::OnGoing) 
        {
            auto valid = game->GetValidMoves();
            if (valid.empty())
            {
                break;
            }

            std::uniform_int_distribution<> randMove(0, static_cast<int>(valid.size()) - 1);
            int move = valid[randMove(gen)];

            game->MakeMove(move);

            float eval = network->Evaluate(game->GetBoardState());

            if (eval < outMinEval) outMinEval = eval;
            if (eval > outMaxEval) outMaxEval = eval;
        }
    }

    std::cout << "[FUZZING COMPLETE] Min Eval: " << outMinEval << ", Max Eval: " << outMaxEval << "\n";

}

void Trainer::ListSaves(IGame* game)
{
    std::string prefix = game->GetName();
    std::string suffix = ".nn";

    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA("./*", &findData);

    if (hFind == INVALID_HANDLE_VALUE) 
    {
        std::cerr << "Error opening directory.\n";
        return;
    }

    std::vector<std::string> matchedFiles;

    do 
    {
        std::string fileName = findData.cFileName;
        if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) 
        {
            if (fileName.size() >= prefix.size() + suffix.size() &&
                fileName.substr(0, prefix.size()) == prefix &&
                fileName.substr(fileName.size() - suffix.size()) == suffix) 
            {
                matchedFiles.push_back(fileName);
            }
        }
    } while (FindNextFileA(hFind, &findData));

    FindClose(hFind);

    if (matchedFiles.empty())
    {
        std::cout << "No saved networks found.\n";
    }
    else 
    {
        std::cout << "Saved networks:\n";
        for (const auto& file : matchedFiles) 
        {
            std::cout << "  - " << file << "\n";
        }
    }
}

void Trainer::ChangeParametersMenu()
{
    while (true)
    {
        std::cout << "\n=== Neural Network Trainer ===\n";
        std::cout << "1. Mutation rate (current: " << m_mutationRate << ")\n";
        std::cout << "2. Population size (current: " << m_populationSize << ")\n";
        std::cout << "3. Matches per iteration (current: " << m_matchesPerIteration << ")\n";
        std::cout << "4. Learning rate (current: " << m_learningRate << ")\n";
        std::cout << "0. Exit\n";
        std::cout << "Choice: ";

        int choice;
        std::cin >> choice;

        if (std::cin.fail()) 
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }

        switch (choice) 
        {
        case 1: 
        {
            std::cout << "Enter new mutation rate (0 or positive int): ";
            int newMutationRate;
            std::cin >> newMutationRate;
            if (!std::cin.fail() && newMutationRate >= 0) 
            {
                m_mutationRate = newMutationRate;
            }
            else 
            {
                std::cout << "Invalid number.\n";
            }
            break;
        }
        case 2: 
        {
            std::cout << "Enter new population size (even number >= 2): ";
            int newSize;
            std::cin >> newSize;
            if (!std::cin.fail() && newSize >= 2 && newSize % 2 == 0) 
            {
                if (newSize < m_population.size()) 
                {
                    m_population.resize(newSize);
                }
                else 
                {
                    while (m_population.size() < newSize) 
                    {
                        m_population.emplace_back(Player{ std::make_unique<NeuralNetwork>(
                            this->m_baseGame->GetBoardState().size(),
                            std::vector<int>{42, 42, 21, 8}
                            ), 0 });
                    }
                }
                m_populationSize = newSize;
                std::cout << "Population resized to " << newSize << ".\n";
            }
            else 
            {
                std::cout << "Invalid size.\n";
            }
            break;
        }
        case 3: 
        {
            std::cout << "Enter new matches per iteration (0 or more): ";
            int newMatches;
            std::cin >> newMatches;
            if (!std::cin.fail() && newMatches >= 0) 
            {
                m_matchesPerIteration = newMatches;
            }
            else 
            {
                std::cout << "Invalid number.\n";
            }
            break;
        }
        case 4: 
        {
            std::cout << "Enter new learning rate (more than 0): ";
            float newRate;
            std::cin >> newRate;
            if (!std::cin.fail() && newRate > 0) 
            {
                m_learningRate = newRate;
            }
            else 
            {
                std::cout << "Invalid number.\n";
            }
            break;
        }
        case 0:
            return;
        default:
            std::cout << "Invalid choice.\n";
        }
    }
}


void Trainer::TrainIterations(int generations) 
{
    std::random_device rd;
    std::mt19937 gen(rd());

    for (int genIndex = 0; genIndex < generations; ++genIndex) 
    {
        for (auto& player : m_population) 
        {
            player.Wins = 0;
            player.Losses = 0;
        }
        for (int i = 0; i < m_populationSize; ++i) 
        {
            for (int m = 0; m < m_matchesPerIteration; ++m)
            {
                std::uniform_int_distribution<> dist(0, m_populationSize - 1);
                int opponentIdx;
                do 
                {
                    opponentIdx = dist(gen);
                } while (opponentIdx == i);

                NeuralNetwork* netA = m_population[i].NN.get();
                NeuralNetwork* netB = m_population[opponentIdx].NN.get();

                bool iIsSecond = dist(gen) % 2 == 0;
                IGame::Winner winner = iIsSecond
                    ? PlayMatch(netB, netA)
                    : PlayMatch(netA, netB);

                if (winner == IGame::Winner::Draw) 
                {
                    
                }
                else if ((iIsSecond && winner == IGame::Winner::SecondPlayer) ||
                    (!iIsSecond && winner == IGame::Winner::FirstPlayer)) 
                {
                    m_population[i].Wins++;
                    m_population[opponentIdx].Losses++;
                }
                else 
                {
                    m_population[i].Losses++;
                    m_population[opponentIdx].Wins++;
                }
            }
        }

        std::sort(m_population.begin(), m_population.end(), [](const Player& a, const Player& b) 
        {
            float aTotal = a.Wins + a.Losses;
            float bTotal = b.Wins + b.Losses;
            float aRatio = aTotal > 0 ? static_cast<float>(a.Wins) / aTotal : 0.0f;
            float bRatio = bTotal > 0 ? static_cast<float>(b.Wins) / bTotal : 0.0f;
            return aRatio > bRatio;
        });

        /*
        std::cout << "Generation " << genIndex << " summary:\n";
        for (int i = 0; i < populationSize; ++i) {
            int wins = population[i].wins;
            int losses = population[i].losses;
            int total = wins + losses;
            float ratio = total > 0 ? static_cast<float>(wins) / total : 0.0f;
            std::cout << "  Player " << population[i].NN->id
                << " | Wins: " << wins
                << " | Losses: " << losses
                << " | Win Ratio: " << ratio << "\n";
        }*/

        float bestTotal = m_population[0].Wins + m_population[0].Losses;
        float bestRatio = bestTotal > 0 ? static_cast<float>(m_population[0].Wins) / bestTotal : 0.0f;

        if (m_championId != m_population[0].NN->Id) 
        {
            m_championId = m_population[0].NN->Id;
            ++m_championImprovements;
            std::cout << "Generation " << genIndex
                << ": new champion! ID: " << m_championId
                << ", Win Ratio: "
                << (bestTotal > 0 ? static_cast<float>(m_population[0].Wins) / bestTotal : 0.0f)
                << "\n";
        }

        int survivors = m_populationSize / 2;
        std::vector<Player> nextGen;

        for (int i = 0; i < survivors; ++i)
        {
            nextGen.push_back(Player{ std::make_unique<NeuralNetwork>(*m_population[i].NN), 0, 0 });
        }

        std::uniform_int_distribution<> survivorDist(0, survivors - 1);
        while (nextGen.size() < m_populationSize) 
        {
            int parentIdx = survivorDist(gen);
            auto childNN = std::make_unique<NeuralNetwork>(
                nextGen[parentIdx].NN->Mutate(m_mutationRate, m_mutationRate));
            childNN->Id = NeuralNetwork::NextId++;
            nextGen.push_back(Player{ std::move(childNN), 0, 0 });
        }

        m_population = std::move(nextGen);
    }

    std::cout << "\nTraining complete. Champion improved "
        << m_championImprovements << " times over "
        << generations << " generations.\n";
    m_championImprovements = 0;
}


NeuralNetwork* Trainer::GetChampion()
{
    for (const auto& player : m_population) 
    {
        if (player.NN->Id == m_championId) 
        {
            return player.NN.get();
        }
    }
    return nullptr;
}


void Trainer::TrainIterationsAgainstRandom(int generations) 
{
    std::random_device rd;
    std::mt19937 gen(rd());

    for (int genIndex = 0; genIndex < generations; ++genIndex) 
    {
        for (auto& player : m_population) 
        {
            player.Wins = 0;
            player.Losses = 0;
        }

        for (auto& player : m_population) 
        {
            for (int m = 0; m < m_matchesPerIteration; ++m) 
            {
                auto game = m_baseGame->Clone();
                std::uniform_int_distribution<> coin(0, 1);
                bool aiPlaysFirst = coin(gen) == 0;

                while (game->GetWinner() == IGame::Winner::OnGoing) 
                {
                    int playerTurn = game->GetCurrentPlayer();
                    int move;

                    if ((playerTurn == 1 && aiPlaysFirst) || (playerTurn == 2 && !aiPlaysFirst)) 
                    {
                        move = ChooseBestMove(*game, player.NN.get());
                    }
                    else 
                    {
                        auto valid = game->GetValidMoves();
                        std::uniform_int_distribution<> randMove(0, static_cast<int>(valid.size()) - 1);
                        move = valid[randMove(gen)];
                    }

                    game->MakeMove(move);
                }

                auto result = game->GetWinner();
                if ((aiPlaysFirst && result == IGame::Winner::FirstPlayer) ||
                    (!aiPlaysFirst && result == IGame::Winner::SecondPlayer)) 
                {
                    ++player.Wins;
                }
                else if (result != IGame::Winner::Draw) 
                {
                    ++player.Losses;
                }
            }
        }

        std::sort(m_population.begin(), m_population.end(), [](const Player& a, const Player& b) 
        {
            float aTotal = a.Wins + a.Losses;
            float bTotal = b.Wins + b.Losses;
            float aRatio = aTotal > 0 ? static_cast<float>(a.Wins) / aTotal : 0.0f;
            float bRatio = bTotal > 0 ? static_cast<float>(b.Wins) / bTotal : 0.0f;
            return aRatio > bRatio;
        });

        int survivors = m_populationSize / 2;
        std::vector<Player> nextGen;

        for (int i = 0; i < survivors; ++i) 
        {
            nextGen.push_back(Player{ std::make_unique<NeuralNetwork>(*m_population[i].NN), 0, 0 });
        }

        std::uniform_int_distribution<> survivorDist(0, survivors - 1);
        while (nextGen.size() < m_populationSize) 
        {
            int parentIdx = survivorDist(gen);
            auto childNN = std::make_unique<NeuralNetwork>(
                nextGen[parentIdx].NN->Mutate(m_mutationRate, m_mutationRate));
            childNN->Id = NeuralNetwork::NextId++;
            nextGen.push_back(Player{ std::move(childNN), 0, 0 });
        }

        bool championAlive = false;
        for (const auto& player : nextGen) 
        {
            if (player.NN->Id == m_championId) 
            {
                championAlive = true;
                break;
            }
        }

        if (!championAlive)
        {
            m_championId = nextGen[0].NN->Id;
            ++m_championImprovements;

            float bestTotal = nextGen[0].Wins + nextGen[0].Losses;
            float bestRatio = bestTotal > 0 ? static_cast<float>(nextGen[0].Wins) / bestTotal : 0.0f;

            std::cout << "Generation " << genIndex
                << ": champion eliminated. New champion ID: " << m_championId << "\n";
        }

        m_population = std::move(nextGen);
    }

    std::cout << "\n[Random Trainer] Champion improved "
        << m_championImprovements << " times over "
        << generations << " generations.\n";
    m_championImprovements = 0;
}


IGame::Winner Trainer::PlayMatch(NeuralNetwork* nn1, NeuralNetwork* nn2) 
{
    auto game = m_baseGame->Clone();

    while (game->GetWinner() == IGame::Winner::OnGoing) 
    {
        NeuralNetwork* currentNN = game->GetCurrentPlayer() == 1 ? nn1 : nn2;
        int move = ChooseBestMove(*game, currentNN);
        game->MakeMove(move);
    }

    return game->GetWinner();
}

int Trainer::ChooseBestMove(const IGame& game, const NeuralNetwork* network) 
{
    std::vector<int> validMoves = game.GetValidMoves();
    int currentPlayer = game.GetCurrentPlayer();

    float bestScore = currentPlayer == 1 ? std::numeric_limits<float>::max() : -std::numeric_limits<float>::max();;
    int bestMove = -1;


    for (int move : validMoves) 
    {
        auto simGame = CloneGameWithMove(game, move);
        float score = network->Evaluate(simGame->GetBoardState());

        //std::cout << "Score: " << score << "\n";
        //std::cout << "Current player: " << simGame->GetCurrentPlayer() << "\n";
        //simGame->PrintBoard();

        if (currentPlayer != 1) 
        {
            if (score > bestScore) 
            {
                bestScore = score;
                bestMove = move;
            }
        }
        else 
        {
            if (score < bestScore) 
            {
                bestScore = score;
                bestMove = move;
            }
        }
    }

    if (bestMove == -1 && !validMoves.empty()) 
    {
        game.PrintBoard();
        std::cerr << "[WARNING] No best move found. Choosing fallback.\n";
        bestMove = validMoves[0];
    }

    return bestMove;
}

void FuzzEvaluationExtremes(const IGame& baseGame, NeuralNetwork* network, int nGames, std::mt19937& gen, float& outMinEval, float& outMaxEval) 
{
    outMinEval = std::numeric_limits<float>::max();
    outMaxEval = std::numeric_limits<float>::lowest();

    for (int i = 0; i < nGames; ++i) 
    {
        auto game = baseGame.Clone();

        while (game->GetWinner() == IGame::Winner::OnGoing) 
        {
            auto valid = game->GetValidMoves();
            if (valid.empty())
            {
                break;
            }

            std::uniform_int_distribution<> randMove(0, static_cast<int>(valid.size()) - 1);
            int move = valid[randMove(gen)];

            game->MakeMove(move);

            float eval = network->Evaluate(game->GetBoardState());

            if (eval < outMinEval)
            {
                outMinEval = eval;
            }
            if (eval > outMaxEval)
            {
                outMaxEval = eval;
            }
        }
    }

    std::cout << "[FUZZING COMPLETE] Min Eval: " << outMinEval << ", Max Eval: " << outMaxEval << "\n";
}

IGame::Winner Trainer::PlayMatchGD(NeuralNetwork* nn1, NeuralNetwork* nn2) {
    auto game = m_baseGame->Clone();
    std::vector<Step> history1, history2;

    while (game->GetWinner() == IGame::Winner::OnGoing) 
    {
        NeuralNetwork* currentNN = game->GetCurrentPlayer() == 1 ? nn1 : nn2;
        auto& history = game->GetCurrentPlayer() == 1 ? history1 : history2;

        int move = ChooseBestMove(*game, currentNN);
        history.push_back({ game->GetBoardState() }); // no target yet
        game->MakeMove(move);
    }

    IGame::Winner winner = game->GetWinner();
    float reward = winner == IGame::Winner::Draw ? 0.0f :
        winner == IGame::Winner::FirstPlayer ? +1.0f : -1.0f;

    ApplyRewards(nn1, history1, reward);
    ApplyRewards(nn2, history2, -reward);
    return winner;
}

void Trainer::ApplyRewards(NeuralNetwork* nn, std::vector<Step>& history, float finalReward) 
{
    float gamma = 0.9f;
    float value = finalReward;
    for (int i = history.size() - 1; i >= 0; --i) 
    {
        history[i].TargetValue = value;
        nn->TrainSingle(history[i].BoardState, value, m_learningRate);
        value *= gamma;
    }
}



void Trainer::TestChampionAgainstRandom(int games) 
{
    NeuralNetwork* ai = GetChampion();
    if (!ai) 
    {
        std::cout << "No champion to test.\n";
        return;
    }

    std::random_device rd;
    std::mt19937 gen(rd());

    int firstWins = 0, firstDraws = 0, firstLosses = 0;
    int secondWins = 0, secondDraws = 0, secondLosses = 0;

    for (int i = 0; i < games; ++i) 
    {
        auto game = m_baseGame->Clone();
        bool aiPlaysFirst = (i < games / 2);

        while (game->GetWinner() == IGame::Winner::OnGoing) 
        {
            int player = game->GetCurrentPlayer();
            int move;

            if ((player == 1 && aiPlaysFirst) || (player == 2 && !aiPlaysFirst)) 
            {
                move = ChooseBestMove(*game, ai);
            }
            else
            {
                auto valid = game->GetValidMoves();
                std::uniform_int_distribution<> randMove(0, valid.size() - 1);
                move = valid[randMove(gen)];
            }

            game->MakeMove(move);
        }

        auto result = game->GetWinner();
        if (aiPlaysFirst) 
        {
            if (result == IGame::Winner::FirstPlayer)
            {
                ++firstWins;
            }
            else if (result == IGame::Winner::Draw)
            {
                ++firstDraws;
            }
            else
            {
                ++firstLosses;
            }
        }
        else 
        {
            if (result == IGame::Winner::SecondPlayer)
            {
                ++secondWins;
            }
            else if (result == IGame::Winner::Draw)
            {
                ++secondDraws;
            }
            else
            {
                ++secondLosses;
            }
        }
    }

    std::cout << "\n=== Champion Test Results vs Random Agent ===\n";

    auto percent = [](int value, int total) -> double 
    {
        return total == 0 ? 0.0 : (value * 100.0 / total);
    };

    int halfGames = games / 2;

    std::cout << "AI as First Player:\n";
    std::cout << "  Wins: " << firstWins << " (" << percent(firstWins, halfGames) << "%)\n";
    std::cout << "  Draws: " << firstDraws << " (" << percent(firstDraws, halfGames) << "%)\n";
    std::cout << "  Losses: " << firstLosses << " (" << percent(firstLosses, halfGames) << "%)\n";

    std::cout << "\nAI as Second Player:\n";
    std::cout << "  Wins: " << secondWins << " (" << percent(secondWins, games - halfGames) << "%)\n";
    std::cout << "  Draws: " << secondDraws << " (" << percent(secondDraws, games - halfGames) << "%)\n";
    std::cout << "  Losses: " << secondLosses << " (" << percent(secondLosses, games - halfGames) << "%)\n";
}


void Trainer::TrainIterationsGD(int generations) 
{
    std::random_device rd;
    std::mt19937 gen(rd());

    for (int genIndex = 0; genIndex < generations; ++genIndex) 
    {
        for (int i = 0; i < m_populationSize; ++i) 
        {
            for (int m = 0; m < m_matchesPerIteration; ++m) 
            {
                std::uniform_int_distribution<> dist(0, m_populationSize - 1);
                int opponentIdx;
                do 
                {
                    opponentIdx = dist(gen);
                } while (opponentIdx == i);

                NeuralNetwork* netA = m_population[i].NN.get();
                NeuralNetwork* netB = m_population[opponentIdx].NN.get();

                bool iIsSecond = dist(gen) % 2 == 0;
                iIsSecond ? PlayMatchGD(netB, netA) : PlayMatchGD(netA, netB);
            }
        }

        EvaluateAndPromoteChampion();

        std::cout << "Finished GD generation " << genIndex << "\n";
    }
}

void Trainer::EvaluateAndPromoteChampion() 
{
    std::sort(m_population.begin(), m_population.end(), [](const Player& a, const Player& b) 
    {
        return a.Wins > b.Wins;
    });

    NeuralNetwork* top = m_population[0].NN.get();
    if (m_championId != top->Id) 
    {
        m_championId = top->Id;
        m_championImprovements++;
        std::cout << "New GD champion: ID " << m_championId << "\n";
    }
}


std::unique_ptr<IGame> Trainer::CloneGameWithMove(const IGame& game, int move) 
{
    std::unique_ptr<IGame> copy = game.Clone();
    copy->MakeMove(move);
    return copy;
}
