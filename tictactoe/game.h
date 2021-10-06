#ifndef GAME_H
#define GAME_H

#include <vector>
#include <QRandomGenerator>
#include <QElapsedTimer>

struct Game
{
public:
    using Score = int;

    enum
    {
        DepthMin = 2,
        CpuTimePerMoveMs = 1000,
    };

    enum ScoreDefines
    {
        UndefinedMin = -10000,
        CpuLose = -1000,
        TooComplex = -1,
        Draw = 0,
        CpuWin = 1000,
        UndefinedMax = 10000,
    };

    enum class PlayerEntity
    {
        None,
        User,
        Cpu,
    };

    enum class UiSign
    {
        None,
        X,
        O,
    };

    struct Position
    {
        Position(int x = -1, int y = -1) : mX(x), mY(y) {}
        int mX;
        int mY;
    };

    using Cell = PlayerEntity;
    using Line = std::vector<Cell>;
    using Grid = std::vector<Line>;

public:
    Game(bool isEasyMode = false, bool isCpuFirst = false, int gridSize = 3)
    {
        mMoves = 0;
        mEasyMode = isEasyMode;
        mCpuFirst = isCpuFirst;
        mWinner = PlayerEntity::None;
        mTurn = isCpuFirst ? PlayerEntity::Cpu : PlayerEntity::User;
        mGrid.clear();
        mGrid.resize(gridSize);
        for (auto& line : mGrid)
            line.resize(gridSize);
    }
    bool UserCanMove(Position p) const
    {
        return GetPlayerAtMove() == Game::PlayerEntity::User &&
                GetWinner() == Game::PlayerEntity::None &&
                mGrid[p.mX][p.mY] == Game::PlayerEntity::None;
    }
    void SetMove(Position p)
    {
        mGrid[p.mX][p.mY] = mTurn;
        mMoves++;
        if (ComputeIsOver(p, mMoves, mWinner))
            mTurn = PlayerEntity::None;
        else
            mTurn = mTurn == PlayerEntity::Cpu ? PlayerEntity::User : PlayerEntity::Cpu;
    }
    void ExecuteCpuMove(Position& p, UiSign& sign)
    {
        p = ComputeCpuMove();
        SetMove(p);
        sign = GetUiSign(p);
    }
    UiSign GetUiSign(Position p) const
    {
        PlayerEntity owner = mGrid[p.mX][p.mY];
        return (owner == PlayerEntity::Cpu) == mCpuFirst ? UiSign::X : UiSign::O;
    }
    bool IsEasyMode() const {return mEasyMode;}
    bool IsCpuFirst() const {return mCpuFirst;}
    PlayerEntity GetPlayerAtMove() const {return mTurn;}
    PlayerEntity GetWinner() const {return mWinner;}
    int GetGridSize() const {return mGrid.size();}

private:
    bool ComputeIsOver(Position last, int moves, PlayerEntity& winner) const;
    Position ComputeCpuMove();
    Position ComputeRandomMove();
    Position ComputeMinMaxBestMove();
    Score ComputeMinMaxScore(Position lastMove, int depth, bool isCpu);

private:
    Grid mGrid;
    PlayerEntity mTurn;
    PlayerEntity mWinner;
    bool mEasyMode;
    bool mCpuFirst;
    int mMoves;

    QElapsedTimer mTimer;
    int mTimePerTree;
    int mDepthMax;

};

#endif // GAME_H
