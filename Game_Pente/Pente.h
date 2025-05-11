#pragma once
#include "IGame.h"
#include <vector>

#define IGAME_API __declspec(dllexport)

class IGAME_API Pente : public IGame {
public:
    Pente();
    Pente(const Pente& other);
    std::unordered_map<int, std::string> GetSpritePaths() const;
    std::vector<std::vector<int>> GetSpriteGrid() const;
    bool InterpretAndMakeMove(const std::string& moveStr);
    void Reset();
    bool MakeMove(int x, int y);
    bool MakeMove(int column);
    bool UnMakeMove();
    Winner GetWinner() const;
    std::vector<int> GetValidMoves() const;
    int GetCurrentPlayer() const;

    void PrintBoard() const;

    std::unique_ptr<IGame> Clone() const;
    std::vector<float> GetBoardState() const;

    std::string GetName() const;

    ~Pente();

private:
    struct Coordinates {
        int x = 0;
        int y = 0;
    };
    int m_boardSize = 19;
    std::vector<std::vector<int>> m_board; // 0 = empty, 1 = player 1, 2 = player 2
    int m_currentPlayer;
    Winner m_winner;
    std::vector<std::vector<Coordinates>> m_moveHistory;
    int m_takesForFirst = 0;
    int m_takesForSecond = 0;

    bool CheckIfMoveLegal(int x, int y);
    bool CheckIfBoardFull();
};