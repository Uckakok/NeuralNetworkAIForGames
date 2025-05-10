#pragma once

#define GAME_API __declspec(dllimport)

#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

class IGame {
public:
    enum Winner {
        OnGoing = -1,
        Draw = 0,
        FirstPlayer = 1,
        SecondPlayer = 2,
    };

    inline IGame() {}
    inline IGame(IGame& other) {}

    virtual std::unordered_map<int, std::string> GetSpritePaths() const = 0;
    virtual std::vector<std::vector<int>> GetSpriteGrid() const = 0;
    virtual std::string GetName() const = 0;
    virtual void Reset() = 0;
    virtual std::vector<int> GetValidMoves() const = 0;
    virtual bool MakeMove(int x, int y) = 0;
    virtual bool MakeMove(int moveId) = 0;
    virtual bool UnMakeMove() = 0;
    virtual Winner GetWinner() const = 0;
    virtual void PrintBoard() const = 0;
    virtual int GetCurrentPlayer() const = 0;

    virtual std::vector<float> GetBoardState() const = 0;
    virtual std::unique_ptr<IGame> Clone() const = 0;
    virtual bool InterpretAndMakeMove(const std::string& moveStr) = 0;

    inline virtual ~IGame() {}
};