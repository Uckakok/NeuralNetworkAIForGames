#include "pch.h"
#include "Checkers.h"
#include <iostream>

Checkers::Checkers() 
{
    Reset();
}

Checkers::Checkers(const Checkers& other) 
{
    m_board = other.m_board;
    m_currentPlayer = other.m_currentPlayer;
    m_winner = other.m_winner;
}

std::unordered_map<int, std::string> Checkers::GetSpritePaths() const
{
    return 
    {
        {0, "Resources/Textures/Checkers/background.png"},
        {1, "Resources/Textures/Checkers/player1.png"},
        {2, "Resources/Textures/Checkers/player2.png"},
        {3, "Resources/Textures/Checkers/player1king.png"},
        {4, "Resources/Textures/Checkers/player2king.png"}
    };
}

std::vector<std::vector<int>> Checkers::GetSpriteGrid() const
{
    std::vector<std::vector<int>> grid(m_rows, std::vector<int>(m_cols, 0));

    for (int x = 0; x < m_rows; ++x)
    {
        for (int y = 0; y < m_cols; ++y)
        {
            int piece = m_board[x][y];

            switch (piece)
            {
            case 1:
                grid[x][y] = 1;
                break;
            case 2:
                grid[x][y] = 2;
                break;
            case 3:
                grid[x][y] = 3;
                break;
            case 4:
                grid[x][y] = 4;
                break;
            default:
                grid[x][y] = 0;
                break;
            }
        }
    }

    return grid;
}

Checkers::~Checkers() 
{
    ;
}

void Checkers::Reset() 
{
    m_board = std::vector<std::vector<int>>(8, std::vector<int>(8, 0));
    m_currentPlayer = 1;
    m_winner = Winner::OnGoing;
    m_moveHistory.clear();

    for (int r = 0; r < 3; ++r) 
    {
        for (int c = (r + 1) % 2; c < 8; c += 2) 
        {
            m_board[r][c] = 2;
        }
    }
    for (int r = 5; r < 8; ++r) 
    {
        for (int c = (r + 1) % 2; c < 8; c += 2) 
        {
            m_board[r][c] = 1;
        }
    }
}

bool Checkers::MakeMove(int x, int y)
{
    if (x < 0 || x >= 8 || y < 0 || y >= 8)
    {
        return false;
    }

    int piece = m_board[x][y];

    if (!m_selectionActive)
    {
        if ((m_currentPlayer == 1 && (piece == 1 || piece == 3)) ||
            (m_currentPlayer == 2 && (piece == 2 || piece == 4)))
        {
            m_selectedRow = x;
            m_selectedCol = y;
            m_selectionActive = true;
        }

        return false;
    }
    else
    {
        int from = m_selectedRow * 8 + m_selectedCol;
        int to = x * 8 + y;
        int moveCode = from * 100 + to;

        std::vector<int> validMoves = GetValidMoves();
        for (int code : validMoves)
        {
            if (code == moveCode)
            {
                m_selectionActive = false;
                m_selectedRow = -1;
                m_selectedCol = -1;
                return MakeMove(moveCode);
            }
        }

        m_selectionActive = false;
        m_selectedRow = -1;
        m_selectedCol = -1;
        return false;
    }
}

bool Checkers::InterpretAndMakeMove(const std::string& moveStr) 
{
    // Supports algebraic notation like "d2-c3" or "e3-g5"
    // Columns: a-h -> 0-7, Rows: 1-8 (from bottom) -> 7-0
    if (moveStr.length() != 5 || moveStr[2] != '-')
    {
        return false;
    }

    auto parsePos = [](char colChar, char rowChar) -> std::pair<int, int> 
    {
        int col = colChar - 'a';
        int row = 8 - (rowChar - '0');
        return std::make_pair(row, col);
    };

    std::pair<int, int> from = parsePos(moveStr[0], moveStr[1]);
    std::pair<int, int> to = parsePos(moveStr[3], moveStr[4]);

    if (from.first < 0 || from.first >= 8 || from.second < 0 || from.second >= 8 ||
        to.first < 0 || to.first >= 8 || to.second < 0 || to.second >= 8) 
    {
        return false;
    }

    int move = (from.first * 8 + from.second) * 100 + (to.first * 8 + to.second);
    return MakeMove(move);
}


bool Checkers::MakeMove(int moveCode) 
{
    if (m_winner != Winner::OnGoing)
    {
        return false;
    }

    int from = moveCode / 100;
    int to = moveCode % 100;
    int fr = from / 8, fc = from % 8;
    int tr = to / 8, tc = to % 8;

    if (fr < 0 || fr >= 8 || fc < 0 || fc >= 8 || tr < 0 || tr >= 8 || tc < 0 || tc >= 8)
    {
        return false;
    }

    if (m_multiCaptureRow != -1 && (fr != m_multiCaptureRow || fc != m_multiCaptureCol))
    {
        return false;
    }

    int piece = m_board[fr][fc];
    if ((m_currentPlayer == 1 && (piece != 1 && piece != 3)) || (m_currentPlayer == 2 && (piece != 2 && piece != 4)))
    {
        return false;
    }

    int dr = tr - fr, dc = tc - fc;
    if (std::abs(dc) != std::abs(dr))
    {
        return false;
    }

    if (m_board[tr][tc] != 0)
    {
        return false;
    }

    bool isKing = (piece == 3 || piece == 4);
    int dir = (m_currentPlayer == 1 ? -1 : 1);
    bool isCapture = false;
    int capturedR = -1, capturedC = -1, capturedPiece = 0;

    if (std::abs(dr) == 1 && !isKing && dr != dir && piece != 3 && piece != 4)
    {
        return false;
    }

    if (std::abs(dr) == 2 && std::abs(dc) == 2) 
    {
        int mr = (fr + tr) / 2, mc = (fc + tc) / 2;
        int midPiece = m_board[mr][mc];
        if (midPiece != 0 && (midPiece % 2) != (piece % 2)) 
        {
            isCapture = true;
            capturedR = mr;
            capturedC = mc;
            capturedPiece = midPiece;
        }
        else 
        {
            return false;
        }
    }
    else if (m_multiCaptureRow != -1)
    {
        return false;
    }

    if (m_multiCaptureRow == -1) 
    {
        std::vector<int> validMoves = GetValidMoves();
        for (int move : validMoves) 
        {
            int f = move / 100, t = move % 100;
            if (std::abs(f / 8 - t / 8) == 2 && !isCapture)
            {
                return false;
            }
        }
    }

    MoveRecord record;
    record.moveCode = moveCode;
    record.capturedR = capturedR;
    record.capturedC = capturedC;
    record.capturedPiece = capturedPiece;
    record.multiCaptureRow = m_multiCaptureRow;
    record.multiCaptureCol = m_multiCaptureCol;
    record.boardState = m_board;
    record.savedPlayer = m_currentPlayer;
    record.savedWinner = m_winner;

    m_board[tr][tc] = piece;
    m_board[fr][fc] = 0;
    if (isCapture) m_board[capturedR][capturedC] = 0;

    if ((m_currentPlayer == 1 && tr == 0 && piece == 1))
    {
        m_board[tr][tc] = 3;
    }
    if ((m_currentPlayer == 2 && tr == 7 && piece == 2))
    {
        m_board[tr][tc] = 4;
    }

    bool willContinueCapture = false;
    if (isCapture) 
    {
        int newPiece = m_board[tr][tc];
        std::vector<std::pair<int, int>> directions = { {1, 1}, {1, -1}, {-1, 1}, {-1, -1} };

        for (const auto& dir : directions) 
        {
            int dr2 = dir.first, dc2 = dir.second;
            int mr = tr + dr2, mc = tc + dc2;
            int tr2 = tr + 2 * dr2, tc2 = tc + 2 * dc2;

            if (tr2 >= 0 && tr2 < 8 && tc2 >= 0 && tc2 < 8 &&
                mr >= 0 && mr < 8 && mc >= 0 && mc < 8 &&
                m_board[tr2][tc2] == 0 &&
                m_board[mr][mc] != 0 &&
                (m_board[mr][mc] % 2 != m_board[tr][tc] % 2)) 
            {
                m_multiCaptureRow = tr;
                m_multiCaptureCol = tc;
                willContinueCapture = true;
                break;
            }
        }
    }

    m_moveHistory.push_back(record);

    if (willContinueCapture)
    {
        return true;
    }

    m_multiCaptureRow = -1;
    m_multiCaptureCol = -1;

    if (GetValidMoves().empty()) 
    {
        m_winner = m_currentPlayer == 1 ? Winner::FirstPlayer : Winner::SecondPlayer;
    }

    m_currentPlayer = 3 - m_currentPlayer;
    return true;
}


bool Checkers::HasFurtherCaptures(int r, int c, int piece) const 
{
    int dir = (piece == 1 || piece == 3) ? -1 : 1;
    std::vector<std::pair<int, int>> directions = (piece == 3 || piece == 4) ?
        std::vector<std::pair<int, int>>{{1, 1}, { 1,-1 }, { -1,1 }, { -1,-1 }} :
        std::vector<std::pair<int, int>>{ {dir,1}, {dir,-1}, {-dir,1}, {-dir,-1} };

    for (const auto& dir : directions) 
    {
        int dr = dir.first;
        int dc = dir.second;
        int mr = r + dr, mc = c + dc;
        int tr = r + 2 * dr, tc = c + 2 * dc;

        if (tr >= 0 && tr < 8 && tc >= 0 && tc < 8 && m_board[tr][tc] == 0) 
        {
            if (mr >= 0 && mr < 8 && mc >= 0 && mc < 8) 
            {
                int mid = m_board[mr][mc];
                if (mid != 0 && (mid % 2 != piece % 2))
                {
                    return true;
                }
            }
        }
    }

    return false;
}

bool Checkers::UnMakeMove() 
{
    if (m_moveHistory.empty())
    {
        return false;
    }

    MoveRecord lastMove = m_moveHistory.back();
    m_moveHistory.pop_back();

    m_board = lastMove.boardState;
    m_currentPlayer = lastMove.savedPlayer;
    m_winner = lastMove.savedWinner;
    m_multiCaptureRow = lastMove.multiCaptureRow;
    m_multiCaptureCol = lastMove.multiCaptureCol;

    return true;
}


std::vector<int> Checkers::GetValidMoves() const 
{
    std::vector<int> moves;
    std::vector<int> captureMoves;
    int dir = (m_currentPlayer == 1 ? -1 : 1);

    if (m_multiCaptureRow != -1) 
    {
        int fr = m_multiCaptureRow;
        int fc = m_multiCaptureCol;
        int piece = m_board[fr][fc];
        std::vector<std::pair<int, int>> directions;

        if (piece == 3 || piece == 4) 
        {
            directions = { {1, 1}, {1, -1}, {-1, 1}, {-1, -1} };
        }
        else 
        { 
            directions = { {dir, 1}, {dir, -1}, {-dir, 1}, {-dir, -1} };
        }

        for (const auto& dir : directions) 
        {
            int dr = dir.first, dc = dir.second;
            int mr = fr + dr, mc = fc + dc;
            int cr = fr + 2 * dr, cc = fc + 2 * dc;

            if (cr >= 0 && cr < 8 && cc >= 0 && cc < 8 &&
                m_board[cr][cc] == 0 &&
                m_board[mr][mc] != 0 &&
                (m_board[mr][mc] % 2 != piece % 2)) 
            {
                int move = (fr * 8 + fc) * 100 + (cr * 8 + cc);
                captureMoves.push_back(move);
            }
        }

        return captureMoves;
    }

    for (int r = 0; r < 8; ++r)
    {
        for (int c = 0; c < 8; ++c) 
        {
            int piece = m_board[r][c];
            if ((m_currentPlayer == 1 && (piece == 1 || piece == 3)) ||
                (m_currentPlayer == 2 && (piece == 2 || piece == 4))) 
            {

                std::vector<std::pair<int, int>> directions;
                if (piece == 3 || piece == 4) 
                {
                    directions = { {1, 1}, {1, -1}, {-1, 1}, {-1, -1} };
                }
                else 
                {
                    directions = { {dir, 1}, {dir, -1}, {-dir, 1}, {-dir, -1} };
                }

                for (const auto& dir : directions) 
                {
                    int dr = dir.first, dc = dir.second;
                    int tr = r + dr, tc = c + dc;

                    if (tr >= 0 && tr < 8 && tc >= 0 && tc < 8 && m_board[tr][tc] == 0) 
                    {
                        int move = (r * 8 + c) * 100 + (tr * 8 + tc);
                        moves.push_back(move);
                    }

                    int cr = r + 2 * dr, cc = c + 2 * dc;
                    int mr = r + dr, mc = c + dc;

                    if (cr >= 0 && cr < 8 && cc >= 0 && cc < 8 &&
                        m_board[cr][cc] == 0 &&
                        m_board[mr][mc] != 0 &&
                        (m_board[mr][mc] % 2 != piece % 2)) 
                    {
                        int move = (r * 8 + c) * 100 + (cr * 8 + cc);
                        captureMoves.push_back(move);
                    }
                }
            }
        }
    }

    return !captureMoves.empty() ? captureMoves : moves;
}


bool Checkers::AnyCapturesAvailable() const 
{
    auto moves = GetValidMoves();
    for (int m : moves) 
    {
        int from = m / 100, to = m % 100;
        int fr = from / 8, fc = from % 8;
        int tr = to / 8, tc = to % 8;
        if (std::abs(fr - tr) == 2)
        {
            return true;
        }
    }
    return false;
}

std::vector<float> Checkers::GetBoardState() const 
{
    std::vector<float> state;
    for (const auto& row : m_board)
    {
        for (int cell : row)
        {
            if (cell == 1)
            {
                state.push_back(1.f);
            }
            else if (cell == 3) 
            {
                state.push_back(1.5f);
            }
            else if (cell == 2)
            {
                state.push_back(-1.f);
            }
            else if (cell == 4)
            {
                state.push_back(-1.5f);
            }
            else
            {
                state.push_back(0.f);
            }
        }
    }
    state.push_back(static_cast<float>(m_currentPlayer));
    return state;
}

std::vector<float> Checkers::GetState() const 
{
    return GetBoardState();
}

Checkers::Winner Checkers::GetWinner() const 
{
    return m_winner;
}

int Checkers::GetCurrentPlayer() const 
{
    return m_currentPlayer;
}

std::string Checkers::GetName() const 
{
    return "Checkers";
}

std::unique_ptr<IGame> Checkers::Clone() const 
{
    return std::make_unique<Checkers>(*this);
}

void Checkers::PrintBoard() const 
{
    std::cout << "  a b c d e f g h\n";
    for (int r = 0; r < 8; ++r) 
    {
        std::cout << (8 - r) << ' '; 
        for (int c = 0; c < 8; ++c) 
        {
            char ch = '.';
            switch (m_board[r][c]) 
            {
            case 1: 
                ch = 'x';
                break;
            case 2: 
                ch = 'o'; 
                break;
            case 3: 
                ch = 'X'; 
                break;
            case 4: 
                ch = 'O'; 
                break;
            }
            std::cout << ch << ' ';
        }
        std::cout << (8 - r) << '\n';  
    }
    std::cout << "  a b c d e f g h\n\n";
}


