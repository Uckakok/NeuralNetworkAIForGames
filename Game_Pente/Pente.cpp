#include "pch.h"
#include "Pente.h"
#include <iostream>

Pente::Pente(const Pente& other)
{
    m_board = other.m_board;
    m_currentPlayer = other.m_currentPlayer;
    m_winner = other.m_winner;
}

std::unordered_map<int, std::string> Pente::GetSpritePaths() const
{
    return
    {
        {0, "Resources/Textures/Pente/background.png"},
        {1, "Resources/Textures/Pente/player1.png"},
        {2, "Resources/Textures/Pente/player2.png"}
    };
}

std::vector<std::vector<int>> Pente::GetSpriteGrid() const
{
    std::vector<std::vector<int>> grid(m_boardSize, std::vector<int>(m_boardSize, 0));

    for (int x = 0; x < m_boardSize; ++x)
    {
        for (int y = 0; y < m_boardSize; ++y)
        {
            switch (m_board[x][y])
            {
            case 1:
                grid[x][y] = 1;
                break;
            case 2:
                grid[x][y] = 2;
                break;
            default:
                grid[x][y] = 0;
                break;
            }
        }
    }

    return grid;
}


bool Pente::InterpretAndMakeMove(const std::string& moveStr)
{
    if (moveStr.length() < 2 || moveStr.length() > 3) 
    {
        return false; 
    }

    char colChar = moveStr[0];
    if (colChar < 'a' || colChar >= 'a' + m_boardSize) 
    {
        return false;
    }

    int x = colChar - 'a';

    std::string rowPart = moveStr.substr(1);
    int y;
    try 
    {
        y = m_boardSize - std::stoi(rowPart);
    }
    catch (...) 
    {
        return false;
    }

    if (y < 0 || y >= m_boardSize) 
    {
        return false;
    }

    int moveCode = y * m_boardSize + x;
    return MakeMove(moveCode);
}

Pente::~Pente()
{
    ;
}

Pente::Pente()
{
    Reset();
}

void Pente::Reset()
{
    m_board = std::vector<std::vector<int>>(m_boardSize, std::vector<int>(m_boardSize, 0));
    m_currentPlayer = 1;
    m_winner = Winner::OnGoing;
    m_moveHistory.clear();
}

bool Pente::MakeMove(int x, int y)
{
    int moveCode = x * m_boardSize + y;
    return MakeMove(moveCode);
}

bool Pente::CheckIfMoveLegal(int x, int y)
{
    if (x < 0 || x >= m_boardSize || y < 0 || y >= m_boardSize) 
    {
        return false;
    }
    if (m_board[y][x] != 0) 
    {
        return false;
    }
    return true;
}

bool Pente::MakeMove(int moveCode)
{
    int x = moveCode % m_boardSize;
    int y = moveCode / m_boardSize;

    if (!CheckIfMoveLegal(x, y)) 
    {
        return false;
    }
    m_board[y][x] = m_currentPlayer;

    int dir[][2] = { { 0, 1 }, {0, -1}, {1, 0}, {-1, 0}, {-1, -1}, {1, 1 }, {-1, 1}, {1, -1} };
    for (int i = 0; i < 4; ++i) 
    {
        int rowCount = 1;
        bool blockedFirstDir = false;
        bool blockedSecondDir = false;
        for (int j = 1; j < 5; ++j) 
        {
            if (x + j * dir[i * 2][1] < 0 || x + j * dir[i * 2][1] >= m_boardSize || y + j * dir[i * 2][0] < 0 || y + j * dir[i * 2][0] >= m_boardSize || blockedFirstDir) {
                ;
            }
            else 
            {
                if (m_board[y + j * dir[i * 2][0]][x + j * dir[i * 2][1]] == m_currentPlayer) 
                {
                    rowCount += 1;
                }
                else 
                {
                    blockedFirstDir = true;
                }
            }

            if (x + j * dir[i * 2 + 1][1] < 0 || x + j * dir[i * 2 + 1][1] >= m_boardSize || y + j * dir[i * 2 + 1][0] < 0 || y + j * dir[i * 2 + 1][0] >= m_boardSize || blockedSecondDir)
            {
                ;
            }
            else 
            {
                if (m_board[y + j * dir[i * 2 + 1][0]][x + j * dir[i * 2 + 1][1]] == m_currentPlayer) 
                {
                    rowCount += 1;
                }
                else 
                {
                    blockedSecondDir = true;
                }
            }
        }
        if (rowCount >= 5) 
        {
            m_winner = static_cast<IGame::Winner>(m_currentPlayer);
        }
    }

    std::vector<Coordinates> moveToSave;
    Coordinates coordToSave;
    coordToSave.x = x;
    coordToSave.y = y;
    moveToSave.push_back(coordToSave);
    for (int i = 0; i < 8; ++i) 
    {
        int dx = dir[i][1];
        int dy = dir[i][0];

        int x1 = x + dx;
        int y1 = y + dy;
        int x2 = x + 2 * dx;
        int y2 = y + 2 * dy;
        int x3 = x + 3 * dx;
        int y3 = y + 3 * dy;

        if (x3 < 0 || x3 >= m_boardSize || y3 < 0 || y3 >= m_boardSize)
        {
            continue;
        }

        if (m_board[y1][x1] == 3 - m_currentPlayer &&
            m_board[y2][x2] == 3 - m_currentPlayer &&
            m_board[y3][x3] == m_currentPlayer) 
        {
            m_board[y1][x1] = 0;
            m_board[y2][x2] = 0;

            if (m_currentPlayer == 1)
            {
                m_takesForFirst += 2;
            }
            else 
            { 
                m_takesForSecond += 2; 
            }

            Coordinates cap1 = { x1, y1 };
            Coordinates cap2 = { x2, y2 };
            moveToSave.push_back(cap1);
            moveToSave.push_back(cap2);
        }
    }

    if (m_takesForFirst >= 10 || m_takesForSecond >= 10)
    {
        m_winner = static_cast<IGame::Winner>(m_currentPlayer);
    }

    m_moveHistory.push_back(moveToSave);

    if (CheckIfBoardFull() && m_winner == IGame::Winner::OnGoing) 
    {
        m_winner = IGame::Winner::Draw;
    }

    m_currentPlayer = 3 - m_currentPlayer;
    return true;
}

bool Pente::UnMakeMove()
{
    if (m_moveHistory.empty())
    {
        return false;
    }

    std::vector<Coordinates> lastMove = m_moveHistory.back();
    m_moveHistory.pop_back();

    m_currentPlayer = 3 - m_currentPlayer;

    Coordinates move = lastMove[0];
    m_board[move.y][move.x] = 0;

    for (size_t i = 1; i < lastMove.size(); ++i) 
    {
        Coordinates cap = lastMove[i];
        m_board[cap.y][cap.x] = 3 - m_currentPlayer;
        if (m_currentPlayer == 1)
        {
            m_takesForFirst--;
        }
        else 
        {
            m_takesForSecond--;
        }
    }

    m_winner = IGame::Winner::OnGoing;
    return true;
}
IGame::Winner Pente::GetWinner() const
{
    return m_winner;
}

std::vector<int> Pente::GetValidMoves() const
{
    std::vector<int> moves;
    if (m_winner != IGame::Winner::OnGoing)
    {
        return moves;
    }

    int radius = 4;
    int center = m_boardSize / 2;
    int startX = std::max(0, center - radius);
    int endX = std::min(m_boardSize - 1, center + radius);
    int startY = std::max(0, center - radius);
    int endY = std::min(m_boardSize - 1, center + radius);

    for (int y = startY; y <= endY; ++y)
    {
        for (int x = startX; x <= endX; ++x)
        {
            if (m_board[y][x] == 0)
            {
                moves.push_back(y * m_boardSize + x);
            }
        }
    }

    return moves;

    /*
    for (int y = 0; y < m_boardSize; ++y)
    {
        for (int x = 0; x < m_boardSize; ++x)
        {
            if (m_board[y][x] == 0)
            {
                moves.push_back(y * m_boardSize + x);
            }
        }
    }

    return moves;
    */
}

int Pente::GetCurrentPlayer() const
{
    return m_currentPlayer;
}

void Pente::PrintBoard() const
{
    printf("  ");
    for (int i = 'a'; i < 't'; ++i)
    {
        printf("%c", i);
    }
    std::cout << std::endl;
    for (int i = 0; i < m_boardSize; ++i) 
    {
        printf("%2d", m_boardSize - i);
        for (int j = 0; j < m_boardSize; ++j) 
        {
            switch (m_board[i][j]) 
            {
            case 0:
                printf(" ");
                break;
            case 1:
                printf("X");
                break;
            case 2:
                printf("0");
                break;
            default:
                system("cls");
                std::cout << "Unexpected error!\n";
            }
        }
        printf("%2d", m_boardSize - i);
        std::cout << std::endl;
    }
    printf("  ");
    for (int i = 'a'; i < 't'; ++i) 
    {
        printf("%c", i);
    }
    std::cout << std::endl;
}

std::vector<float> Pente::GetBoardState() const
{
    std::vector<float> state;
    for (const auto& row : m_board)
    {
        for (int cell : row)
        {
            state.push_back(static_cast<float>(cell));
        }
    }
    state.push_back(m_takesForFirst);
    state.push_back(m_takesForSecond);
    state.push_back(m_currentPlayer);
    return state;
}

std::string Pente::GetName() const
{
    return std::string("Pente");
}

bool Pente::CheckIfBoardFull()
{
    for (int i = 0; i < m_boardSize; ++i) 
    {
        for (int j = 0; j < m_boardSize; ++j) 
        {
            if (m_board[i][j] == 0) return false;
        }
    }
    return true;
}

std::unique_ptr<IGame> Pente::Clone() const
{
    return std::make_unique<Pente>(*this);
}



