#include "Selector.h"
#include "Trainer.h"
#include "MonteCarlo.h"
#include "GraphicHandler.h"
#include <filesystem>
#include <iostream>
#include <string>
#include <limits>


void GameSelector::LoadGameDLLs() 
{
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    std::string exePath(buffer);
    size_t lastSlash = exePath.find_last_of("\\/");
    std::string exeDir = exePath.substr(0, lastSlash);

    std::string dllDir = exeDir + "/";
    std::string searchPattern = dllDir + "*.dll";

    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA(searchPattern.c_str(), &findData);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        return;
    }

    do 
    {
        std::string fullPath = dllDir + findData.cFileName;
        HMODULE lib = LoadLibraryA(fullPath.c_str());
        if (!lib)
        {
            continue;
        }

        auto getName = (GetGameNameFunc)GetProcAddress(lib, "GetGameName");
        auto createGame = (CreateGameFunc)GetProcAddress(lib, "CreateGame");

        if (getName && createGame) 
        {
            m_loadedGames.push_back({ lib, getName(), createGame });
        }
        else 
        {
            FreeLibrary(lib);
        }

    } while (FindNextFileA(hFind, &findData));

    FindClose(hFind);
}

void GameSelector::Start() 
{
    LoadGameDLLs();

    while (true) 
    {
        std::cout << "=== Select a Game ===\n";
        for (size_t i = 0; i < m_loadedGames.size(); ++i) 
        {
            std::cout << i + 1 << ". " << m_loadedGames[i].Name << "\n";
        }
        std::cout << "0. Exit\nChoice: ";

        int choice;
        std::cin >> choice;

        if (std::cin.fail()) 
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input. Please enter a number.\n";
            continue;
        }

        if (choice == 0)
        {
            return;
        }
        if (choice < 1 || choice >(int)m_loadedGames.size()) 
        {
            std::cout << "Invalid choice.\n";
            continue;
        }

        auto& gameEntry = m_loadedGames[choice - 1];

        int mode = -1;
        while (true) 
        {
            std::cout << "1. Hot seat\n2. Neural network trainer\n3. Play against AI\n0. Go Back\nChoice: ";
            std::cin >> mode;

            if (std::cin.fail()) 
            {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "Invalid input. Please enter a number.\n";
                continue;
            }

            if (mode >= 0 && mode <= 4)
            {
                break;
            }

            std::cout << "Invalid choice.\n";
        }

        if (mode == 0)
        {
            continue;
        }

        std::unique_ptr<IGame> game(gameEntry.CreateFunc());

        switch (mode) 
        {
        case 1:
            PlayHotSeat(std::move(game));
            break;

        case 2: 
        {
            Trainer train(game->Clone());
            train.Run();
            break;
        }

        case 3:
            PlayAgainstAI(std::move(game));
            break;
        }
    }
}

void GameSelector::PlayGameLoop(std::unique_ptr<IGame> game, NeuralNetwork* aiNetwork, int humanPlayer) 
{
    bool graphicsMode = true;
    {
        std::unique_ptr<GraphicalInterface> graphics;

        while (game->GetWinner() == IGame::Winner::OnGoing) 
        {
            int current = game->GetCurrentPlayer();

            if (graphicsMode) 
            {
                if (!graphics) 
                {
                    graphics = std::make_unique<GraphicalInterface>(*game);
                }

                if (!graphics->WindowUpdate()) 
                {
                    graphics.reset();
                    graphicsMode = false;
                    continue;
                }

                if (aiNetwork && current != humanPlayer) 
                {
                    int bestMove = MonteCarlo::MonteCarloTreeSearch(*game, 3.0f, aiNetwork);
                    game->MakeMove(bestMove);
                    graphics->SubmitEntitiesFromGrid(game->GetSpriteGrid());
                }

            }
            else 
            {
                game->PrintBoard();
                std::cout << "Enter move (or type 'graphics' to reopen window):\n> ";

                std::string input;
                std::getline(std::cin, input);

                if (input == "graphics") 
                {
                    graphicsMode = true;
                    continue;
                }

                if (input == "back") 
                {
                    game->UnMakeMove();
                    continue;
                }

                if (!game->InterpretAndMakeMove(input)) 
                {
                    std::cout << "Invalid move.\n";
                }
            }
        }
    }

    game->PrintBoard();
    auto winner = game->GetWinner();
    if (winner == IGame::Draw) 
    {
        std::cout << "It's a draw!\n";
    }
    else 
    {
        std::cout << "Player " << (winner == IGame::FirstPlayer ? 1 : 2) << " wins!\n";
    }

    std::cout << "Press Enter to return to menu...\n";
    std::cin.ignore();
    std::cin.get();
}


void GameSelector::PlayHotSeat(std::unique_ptr<IGame> game) 
{
    std::cout << "\n=== " << game->GetName() << " (Hot Seat) ===\n";
    PlayGameLoop(std::move(game), nullptr, 0);
}

void GameSelector::PlayAgainstAI(std::unique_ptr<IGame> game) 
{
    Trainer::ListSaves(game.get());

    std::cout << "Enter name of save to load: ";
    std::string name;
    std::cin >> name;

    if (std::cin.fail()) 
    {
        std::cout << "Failed to read name.\n";
        return;
    }

    try
    {
        NeuralNetwork loaded = NeuralNetwork::Load(name);
        if (!loaded.ClampedEvaluationPossible())
        {
            std::cout << "Bounds aren't set for this neural network! Fix in trainer mode!\n";
            return;
        }
        std::cout << "Loaded network.\n";

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

        PlayGameLoop(std::move(game), &loaded, userPlayer);
    }
    catch (...)
    {
        std::cout << "Failed to load network: " << name << "\n";
    }
}



