#pragma once
#include "IGame.h"
#include <vector>

#define IGAME_API __declspec(dllexport)

class IGAME_API Checkers : public IGame {
public:
    Checkers();
    Checkers(const Checkers& other);
    std::unordered_map<int, std::string> GetSpritePaths() const;
    std::vector<std::vector<int>> GetSpriteGrid() const;
    bool InterpretAndMakeMove(const std::string& moveStr);
    void Reset();
    bool MakeMove(int x, int y);
    bool MakeMove(int moveIndex);
    bool UnMakeMove();
    Winner GetWinner() const;
    std::vector<int> GetValidMoves() const;
    std::vector<float> GetState() const;
    int GetCurrentPlayer() const;

    void PrintBoard() const;

    std::unique_ptr<IGame> Clone() const;
    std::vector<float> GetBoardState() const;

    std::string GetName() const;

    ~Checkers();

private:
    struct MoveRecord {
        int moveCode;
        int capturedR;
        int capturedC;
        int capturedPiece;
        int multiCaptureRow;
        int multiCaptureCol;
        std::vector<std::vector<int>> boardState;
        int savedPlayer;
        Winner savedWinner;
    };

    bool m_selectionActive = false;
    int m_multiCaptureRow = -1;
    int m_multiCaptureCol = -1;
    int m_selectedRow = 0;
    int m_selectedCol = 0;
    static const int m_rows = 8;
    static const int m_cols = 8;
    std::vector<std::vector<int>> m_board;
    int m_currentPlayer;
    Winner m_winner;
    std::vector<MoveRecord> m_moveHistory;

    bool IsMoveCapture(int move);
};