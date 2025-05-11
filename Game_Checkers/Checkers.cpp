#include "pch.h"
#include "Checkers.h"
#include <iostream>
#include <algorithm>

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

    int from = moveCode / 100, to = moveCode % 100;
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
    bool isKing = (piece == 3 || piece == 4);
    if ((m_currentPlayer == 1 && (piece != 1 && piece != 3)) ||
        (m_currentPlayer == 2 && (piece != 2 && piece != 4)))
    {
        return false;
    }

    int dr = tr - fr, dc = tc - fc;
    if (std::abs(dr) != std::abs(dc))
    {
        return false;
    }

    bool isCapture = false;
    int capR = -1, capC = -1, capPiece = 0;

    if (isKing)
    {
        int steps = std::abs(dr);
        int drStep = dr / steps, dcStep = dc / steps;

        int enemyCount = 0;
        for (int s = 1; s < steps; ++s)
        {
            int r = fr + s * drStep, c = fc + s * dcStep;
            if (m_board[r][c] != 0)
            {
                if ((m_board[r][c] % 2) == (piece % 2))
                {
                    return false;
                }
                if (++enemyCount > 1)
                {
                    return false;
                }
                capR = r; capC = c; capPiece = m_board[r][c];
            }
        }

        if (m_board[tr][tc] != 0)
        {
            return false;
        }

        if (enemyCount == 1)
        {
            isCapture = true;
        }
        else if (enemyCount == 0)
        {
            if (m_multiCaptureRow != -1)
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }
    else
    {
        if (std::abs(dr) == 1)
        {
            if (m_multiCaptureRow != -1) return false;
            if (m_board[tr][tc] != 0)       return false;
            if ((m_currentPlayer == 1 && dr != -1) ||
                (m_currentPlayer == 2 && dr != 1))
            {
                return false;
            }
        }
        else if (std::abs(dr) == 2)
        {
            int mr = (fr + tr) / 2, mc = (fc + tc) / 2;
            int mid = m_board[mr][mc];
            if (mid == 0 || (mid % 2) == (piece % 2) || m_board[tr][tc] != 0)
            {
                return false;
            }
            isCapture = true;
            capR = mr; capC = mc; capPiece = mid;
        }
        else
        {
            return false;
        }
    }

    if (m_multiCaptureRow == -1)
    {
        auto allMoves = GetValidMoves();
        bool existsCapture = std::any_of(allMoves.begin(), allMoves.end(), [&](int mv) {
            return IsMoveCapture(mv);
        });
        if (existsCapture && !isCapture)
        {
            return false;
        }
    }

    MoveRecord rec{ moveCode, capR, capC, capPiece,
                    m_multiCaptureRow, m_multiCaptureCol,
                    m_board, m_currentPlayer, m_winner };
    m_moveHistory.push_back(rec);

    m_board[fr][fc] = 0;
    m_board[tr][tc] = piece;
    if (isCapture)
    {
        m_board[capR][capC] = 0;
    }

    if ((m_currentPlayer == 1 && tr == 0 && piece == 1) ||
        (m_currentPlayer == 2 && tr == 7 && piece == 2))
    {
        m_board[tr][tc] += 2;
    }

    bool more = false;
    if (isCapture)
    {
        int saveRow = m_multiCaptureRow, saveCol = m_multiCaptureCol;
        m_multiCaptureRow = tr; m_multiCaptureCol = tc;
        auto next = GetValidMoves();
        m_multiCaptureRow = saveRow; m_multiCaptureCol = saveCol;
        more = !next.empty();
    }

    if (more)
    {
        m_multiCaptureRow = tr;
        m_multiCaptureCol = tc;
    }
    else
    {
        m_multiCaptureRow = m_multiCaptureCol = -1;
        m_currentPlayer = 3 - m_currentPlayer;

        bool oppHasPiece = false;
        for (auto& row : m_board)
        {
            for (int p : row)
            {
                if (p != 0 && ((p % 2) == (m_currentPlayer % 2)))
                {
                    oppHasPiece = true;
                }
            }
        }

        bool oppHasMoves = !GetValidMoves().empty();

        if (!oppHasPiece || !oppHasMoves)
        {
            m_winner = (m_currentPlayer == 1 ? Winner::FirstPlayer : Winner::SecondPlayer);
        }
    }

    return true;
}

bool Checkers::IsMoveCapture(int move) 
{
    int from = move / 100;
    int to = move % 100;
    int fr = from / 8, fc = from % 8;
    int tr = to / 8, tc = to % 8;
    int dr = (tr - fr) > 0 ? 1 : -1;
    int dc = (tc - fc) > 0 ? 1 : -1;
    int r = fr + dr, c = fc + dc;
    while (r != tr && c != tc)
    {
        if (m_board[r][c] != 0 && (m_board[r][c] % 2) != (m_board[fr][fc] % 2))
        {
            return true;
        }
        else if (m_board[r][c] != 0)
        {
            break;
        }
        r += dr;
        c += dc;
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
    std::vector<int> moves, caps;
    int forwardDir = (m_currentPlayer == 1 ? -1 : 1);

    auto trySlide = [&](int fr, int fc, int dr, int dc) 
    {
        for (int s = 1;; ++s) 
        {
            int tr = fr + s * dr, tc = fc + s * dc;
            if (tr < 0 || tr >= 8 || tc < 0 || tc >= 8)
            {
                break;
            }
            if (m_board[tr][tc] != 0)
            {
                break;
            }
            moves.push_back((fr * 8 + fc) * 100 + (tr * 8 + tc));
        }
    };

    if (m_multiCaptureRow != -1)
    {
        int fr = m_multiCaptureRow, fc = m_multiCaptureCol;
        int piece = m_board[fr][fc];
        bool king = (piece == 3 || piece == 4);
        std::vector<std::pair<int, int>> dirs = { {1,1}, {1,-1}, {-1,1}, {-1,-1} };

        for (const auto& dir : dirs)
        {
            int dr = dir.first, dc = dir.second;
            if (!king)
            {
                int mr = fr + dr, mc = fc + dc;
                int tr = fr + 2 * dr, tc = fc + 2 * dc;
                if (tr < 0 || tr >= 8 || tc < 0 || tc >= 8)
                {
                    continue;
                }
                int mid = m_board[mr][mc];
                if (m_board[tr][tc] == 0 && mid != 0 && (mid % 2) != (piece % 2))
                {
                    caps.push_back((fr * 8 + fc) * 100 + (tr * 8 + tc));
                }
            }
            else
            {
                for (int s = 1;; ++s)
                {
                    int r = fr + s * dr, c = fc + s * dc;
                    if (r < 0 || r >= 8 || c < 0 || c >= 8)
                    {
                        break;
                    }
                    if (m_board[r][c] == 0)
                    {
                        continue;
                    }
                    if ((m_board[r][c] % 2) == (piece % 2))
                    {
                        break;
                    }

                    for (int t = 1;; ++t)
                    {
                        int tr = r + t * dr, tc = c + t * dc;
                        if (tr < 0 || tr >= 8 || tc < 0 || tc >= 8)
                        {
                            break;
                        }
                        if (m_board[tr][tc] != 0)
                        {
                            break;
                        }
                        caps.push_back((fr * 8 + fc) * 100 + (tr * 8 + tc));
                    }
                    break;
                }
            }
        }
        return caps;
    }

    for (int r = 0; r < 8; ++r) for (int c = 0; c < 8; ++c)
    {
        int piece = m_board[r][c];
        if (piece == 0 || ((piece % 2) != (m_currentPlayer % 2)))
        {
            continue;
        }

        bool king = (piece == 3 || piece == 4);
        std::vector<std::pair<int, int>> dirs = king ?
            std::vector<std::pair<int, int>>{{1, 1}, { 1,-1 }, { -1,1 }, { -1,-1 }} :
            std::vector<std::pair<int, int>>{ {forwardDir,1},{forwardDir,-1},{-forwardDir,1},{-forwardDir,-1} };

        for (const auto& dir : dirs)
        {
            int dr = dir.first, dc = dir.second;
            if (!king)
            {
                int mr = r + dr, mc = c + dc, tr = r + 2 * dr, tc = c + 2 * dc;
                if (tr < 0 || tr >= 8 || tc < 0 || tc >= 8)
                {
                    continue;
                }
                int mid = m_board[mr][mc];
                if (m_board[tr][tc] == 0 && mid != 0 && (mid % 2) != (piece % 2))
                {
                    caps.push_back((r * 8 + c) * 100 + (tr * 8 + tc));
                }
            }
            else
            {
                for (int s = 1;; ++s)
                {
                    int cr = r + s * dr, cc = c + s * dc;
                    if (cr < 0 || cr >= 8 || cc < 0 || cc >= 8)
                    {
                        break;
                    }
                    if (m_board[cr][cc] == 0)
                    {
                        continue;
                    }
                    if ((m_board[cr][cc] % 2) == (piece % 2))
                    {
                        break;
                    }

                    for (int t = 1;; ++t)
                    {
                        int tr = cr + t * dr, tc = cc + t * dc;
                        if (tr < 0 || tr >= 8 || tc < 0 || tc >= 8)
                        {
                            break;
                        }
                        if (m_board[tr][tc] != 0)
                        {
                            break;
                        }
                        caps.push_back((r * 8 + c) * 100 + (tr * 8 + tc));
                    }
                    break;
                }
            }
        }
    }

    if (!caps.empty())
    {
        return caps;
    }

    for (int r = 0; r < 8; ++r) for (int c = 0; c < 8; ++c)
    {
        int piece = m_board[r][c];
        if (piece == 0 || ((piece % 2) != (m_currentPlayer % 2)))
        {
            continue;
        }

        bool king = (piece == 3 || piece == 4);
        if (!king)
        {
            int tr = r + forwardDir, tc = c + 1;
            if (tr >= 0 && tr < 8 && tc >= 0 && tc < 8 && m_board[tr][tc] == 0)
            {
                moves.push_back((r * 8 + c) * 100 + (tr * 8 + tc));
            }
            tc = c - 1;
            if (tr >= 0 && tr < 8 && tc >= 0 && tc < 8 && m_board[tr][tc] == 0)
            {
                moves.push_back((r * 8 + c) * 100 + (tr * 8 + tc));
            }
        }
        else
        {
            std::vector<std::pair<int, int>> dirs = { {1,1},{1,-1},{-1,1},{-1,-1} };
            for (const auto& dir : dirs)
            {
                trySlide(r, c, dir.first, dir.second);
            }
        }
    }

    return moves;
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


