#include "pch.h"
#include "ConnectFour.h"
#include <iostream>

ConnectFour::ConnectFour(const ConnectFour& other)
{
    m_board = other.m_board;                      
    m_currentPlayer = other.m_currentPlayer;      
    m_winner = other.m_winner;                    
}

std::unordered_map<int, std::string> ConnectFour::GetSpritePaths() const
{
    return 
    {
        {0, "Resources/Textures/Connect4/background.png"},
        {1, "Resources/Textures/Connect4/player1.png"},
        {2, "Resources/Textures/Connect4/player2.png"}
    };
}

std::vector<std::vector<int>> ConnectFour::GetSpriteGrid() const
{
    std::vector<std::vector<int>> grid(m_rows, std::vector<int>(m_cols, 0));

    for (int x = 0; x < m_rows; ++x)
    {
        for (int y = 0; y < m_cols; ++y)
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


bool ConnectFour::InterpretAndMakeMove(const std::string& moveStr)
{
    try 
    {
        int col = std::stoi(moveStr);
        return MakeMove(col);
    }
    catch (...) 
    {
        return false;
    }
}

ConnectFour::~ConnectFour() 
{
    ;
}

ConnectFour::ConnectFour() 
{

    Reset();
}

void ConnectFour::Reset() 
{
    m_board = std::vector<std::vector<int>>(m_rows, std::vector<int>(m_cols, 0));
    m_currentPlayer = 1;
    m_winner = Winner::OnGoing; 
    m_moveHistory.clear();
}

bool ConnectFour::MakeMove(int x, int y) 
{
    if (x < 0 || x >= m_rows || y < 0 || y >= m_cols) 
    {
        return false;
    }

    if (m_board[0][y] != 0) 
    {
        return false;
    }

    for (int row = m_rows - 1; row >= 0; --row) 
    {
        if (m_board[row][y] == 0) 
        {
            m_board[row][y] = m_currentPlayer;

            if (CheckWin(row, y)) 
            {
                m_winner = static_cast<Winner>(m_currentPlayer);
            }
            else if (IsBoardFull()) 
            {
                m_winner = Winner::Draw;
            }
            m_currentPlayer = 3 - m_currentPlayer;

            m_moveHistory.push_back(y);
            return true;
        }
    }

    return false;
}

bool ConnectFour::MakeMove(int column) 
{
    if (column < 0 || column >= m_cols || m_winner != Winner::OnGoing)
    {
        return false;
    }

    for (int row = m_rows - 1; row >= 0; --row) 
    {
        if (m_board[row][column] == 0)
        {
            m_board[row][column] = m_currentPlayer;
            if (CheckWin(row, column)) 
            {
                m_winner = static_cast<Winner>(m_currentPlayer);
            }
            else if (IsBoardFull()) 
            {
                m_winner = Winner::Draw;
            }
            m_currentPlayer = 3 - m_currentPlayer;
            m_moveHistory.push_back(column);
            return true;
        }
    }

    return false;
}

bool ConnectFour::UnMakeMove()
{
    if (m_moveHistory.empty()) 
    {
        return false;
    }

    int lastMoveColumn = m_moveHistory.back();
    m_moveHistory.pop_back();

    int row = -1;
    for (int i = 0; i < m_rows; ++i) 
    {
        if (m_board[i][lastMoveColumn] != 0) 
        {
            row = i;
            break;
        }
    }

    if (row == -1) 
    {
        return false;
    }

    m_board[row][lastMoveColumn] = 0;
    m_currentPlayer = 3 - m_currentPlayer;
    m_winner = Winner::OnGoing;
    return true;
}

IGame::Winner ConnectFour::GetWinner() const 
{
    return m_winner;
}

std::vector<int> ConnectFour::GetValidMoves() const 
{
    std::vector<int> moves;
    if (m_winner != IGame::Winner::OnGoing)
    {
        return moves;
    }
    for (int col = 0; col < m_cols; ++col) 
    {
        if (m_board[0][col] == 0) 
        {
            moves.push_back(col);
        }
    }
    return moves;
}

int ConnectFour::GetCurrentPlayer() const 
{
    return m_currentPlayer;
}

std::vector<float> ConnectFour::GetState() const 
{
    std::vector<float> state;
    for (const auto& row : m_board) 
    {
        for (int cell : row) 
        {
            state.push_back(static_cast<float>(cell));
        }
    }
    return state;
}

bool ConnectFour::IsBoardFull() const 
{
    for (int col = 0; col < m_cols; ++col)
    {
        if (m_board[0][col] == 0)
        {
            return false;
        }
    }
    return true;
}

bool ConnectFour::CheckWin(int row, int col) 
{
    int player = m_board[row][col];
    if (player == 0)
    {
        return false;
    }

    static const int directions[4][2] = {
        {0, 1}, 
        {1, 0}, 
        {1, 1}, 
        {1, -1} 
    };

    for (int i = 0; i < 4; ++i) 
    {
        int dr = directions[i][0];
        int dc = directions[i][1];
        int count = 1;

        int r = row + dr;
        int c = col + dc;
        while (r >= 0 && r < m_rows && c >= 0 && c < m_cols && m_board[r][c] == player) 
        {
            count++;
            r += dr;
            c += dc;
        }

        r = row - dr;
        c = col - dc;
        while (r >= 0 && r < m_rows && c >= 0 && c < m_cols && m_board[r][c] == player)
        {
            count++;
            r -= dr;
            c -= dc;
        }

        if (count >= 4)
        {
            return true;
        }
    }

    return false;
}



void ConnectFour::PrintBoard() const 
{
    for (const auto& row : m_board) 
    {
        for (int cell : row)
        {
            char symbol = '.';
            if (cell == 1) symbol = 'X';
            else if (cell == 2) symbol = 'O';
            std::cout << symbol << ' ';
        }
        std::cout << '\n';
    }
    std::cout << "0 1 2 3 4 5 6\n\n";
}

std::vector<float> ConnectFour::GetBoardState() const 
{
    std::vector<float> state(m_rows * m_cols, 0.0f);
    for (int r = 0; r < m_rows; ++r) 
    {
        for (int c = 0; c < m_cols; ++c) 
        {
            int index = r * m_cols + c;
            if (m_board[r][c] == 1) 
            {
                state[index] = 1.0f;
            }
            else if (m_board[r][c] == 2) 
            {
                state[index] = -1.0f;
            }
        }
    }
    state.push_back(static_cast<float>(m_currentPlayer));
    return state;
}

std::string ConnectFour::GetName() const
{
    return std::string("ConnectFour");
}

std::unique_ptr<IGame> ConnectFour::Clone() const 
{
    return std::make_unique<ConnectFour>(*this);
}



