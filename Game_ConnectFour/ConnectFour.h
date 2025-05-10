#pragma once
#include "IGame.h"
#include <vector>

#define IGAME_API __declspec(dllexport)

class IGAME_API ConnectFour : public IGame {
public:
    ConnectFour();
    ConnectFour(const ConnectFour& other);
    std::unordered_map<int, std::string> GetSpritePaths() const;
    std::vector<std::vector<int>> GetSpriteGrid() const;
    bool InterpretAndMakeMove(const std::string& moveStr);
    void Reset();
    bool MakeMove(int x, int y);
    bool MakeMove(int column);
    bool UnMakeMove();
    Winner GetWinner() const;
    std::vector<int> GetValidMoves() const;
    std::vector<float> GetState() const;
    int GetCurrentPlayer() const;

    void PrintBoard() const;

    std::unique_ptr<IGame> Clone() const;
    std::vector<float> GetBoardState() const;

    std::string GetName() const;

    ~ConnectFour();

private:
    static const int m_rows = 6;
    static const int m_cols = 7;
    std::vector<std::vector<int>> m_board; // 0 = empty, 1 = player 1, 2 = player 2
    int m_currentPlayer;
    Winner m_winner;
    std::vector<int> m_moveHistory;

    bool CheckWin(int lastRow, int lastCol);
    bool IsBoardFull() const;
};