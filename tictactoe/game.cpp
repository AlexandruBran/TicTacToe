#include "game.h"

bool Game::ComputeIsOver(Position last, int moves, PlayerEntity& winner) const
{
    winner = PlayerEntity::None;
    int lastX = last.mX;
    int lastY = last.mY;

    //row
    winner = mGrid[0][lastY];
    for(int i = 1; i < GetGridSize(); i++)
        if (mGrid[i][lastY] == PlayerEntity::None || mGrid[i][lastY] != winner)
        {
            winner = PlayerEntity::None;
            break;
        }

    if (winner != PlayerEntity::None)
        return true;

    //line
    winner = mGrid[lastX][0];
    for(int i = 1; i < GetGridSize(); i++)
        if (mGrid[lastX][i] == PlayerEntity::None || mGrid[lastX][i] != winner)
        {
            winner = PlayerEntity::None;
            break;
        }

    if (winner != PlayerEntity::None)
        return true;

    //diagonal1
    if (lastX == lastY)
    {
        winner = mGrid[0][0];
        for(int i = 1; i < GetGridSize(); i++)
            if (mGrid[i][i] == PlayerEntity::None || mGrid[i][i] != winner)
            {
                winner = PlayerEntity::None;
                break;
            }

        if (winner != PlayerEntity::None)
            return true;
    }

    //diagonal2
    if (lastX == GetGridSize() - lastY - 1)
    {
        winner = mGrid[0][GetGridSize() - 1];
        for(int i = 1; i < GetGridSize(); i++)
            if (mGrid[i][GetGridSize() - i - 1] == PlayerEntity::None || mGrid[i][GetGridSize() - i - 1] != winner)
            {
                winner = PlayerEntity::None;
                break;
            }

        if (winner != PlayerEntity::None)
            return true;
    }

    return moves == (GetGridSize() * GetGridSize());
}

Game::Position Game::ComputeCpuMove()
{
    Game::Position p;

    if (mMoves == 0)
        p.mX = p.mY = (GetGridSize() - 1) / 2;
    else if (mEasyMode)
        p = ComputeRandomMove();
    else
        p = ComputeMinMaxBestMove();

    return p;
}

Game::Position Game::ComputeRandomMove()
{
    Game::Position p;

    int remaining = GetGridSize() * GetGridSize() - mMoves;
    int selected = remaining == 1 ? 0 : QRandomGenerator::global()->bounded(remaining);

    for (int i = 0; i != GetGridSize(); i++)
        for (int j = 0; j != GetGridSize(); j++)
            if (mGrid[i][j] == PlayerEntity::None && selected-- == 0)
            {
                p.mX = i;
                p.mY = j;
            }

    assert(p.mX != -1 && p.mY != -1);
    return p;
}

Game::Position Game::ComputeMinMaxBestMove()
{
    Score bestScore = ScoreDefines::UndefinedMin;
    Game::Position bestMove;

    std::vector<Position> undefinedMoves;

    mTimePerTree = CpuTimePerMoveMs / (GetGridSize() * GetGridSize() - mMoves);

    for (int i = 0; i < GetGridSize(); i++)
    {
        for (int j = 0; j < GetGridSize(); j++)
        {
            auto& cell = mGrid[i][j];
            if (cell != PlayerEntity::None)
                continue;

            cell = PlayerEntity::Cpu;
            mTimer.start();
            Score currentScore = ComputeMinMaxScore({i, j}, 1, false);
            cell = PlayerEntity::None;

            if (currentScore > bestScore || bestMove.mX == -1)
            {
                bestMove = {i, j};
                bestScore = currentScore;
            }

            if (currentScore == TooComplex)
                undefinedMoves.push_back({i, j});
        }
    }

    assert(std::abs(bestScore) <= ScoreDefines::CpuWin);

    if (bestScore == TooComplex)
        return undefinedMoves[QRandomGenerator::global()->bounded((int)undefinedMoves.size())];

    return bestMove;
}

Game::Score Game::ComputeMinMaxScore(Position lastMove, int depth, bool isCpu)
{
    if (depth > DepthMin && mTimer.hasExpired(mTimePerTree))
        return ScoreDefines::TooComplex;

    Game::PlayerEntity winner;
    if (ComputeIsOver(lastMove, mMoves + depth, winner))
    {
        switch (winner)
        {
        case Game::PlayerEntity::None: return ScoreDefines::Draw;
        case Game::PlayerEntity::Cpu: return ScoreDefines::CpuWin - depth;
        case Game::PlayerEntity::User: return ScoreDefines::CpuLose + depth;
        }
    }

    Score bestScore = isCpu ? ScoreDefines::UndefinedMin : ScoreDefines::UndefinedMax;

    for (int i = 0; i < GetGridSize(); i++)
    {
        for (int j = 0; j < GetGridSize(); j++)
        {
            auto& cell = mGrid[i][j];
            if (cell != PlayerEntity::None)
                continue;

            cell = isCpu ? PlayerEntity::Cpu : PlayerEntity::User;
            Score currentScore = ComputeMinMaxScore({i, j}, depth + 1, !isCpu);
            bestScore = isCpu ? std::max(bestScore, currentScore) : std::min(bestScore, currentScore);
            cell = PlayerEntity::None;
        }
    }

    assert(std::abs(bestScore) <= ScoreDefines::CpuWin);
    return bestScore;
}

