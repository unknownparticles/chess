#ifndef GOMOKU_ENGINE_H
#define GOMOKU_ENGINE_H

#include "engine.h"
#include <vector>

class GomokuEngine : public GameEngine {
public:
  GomokuEngine();
  std::vector<GameMove> getPossibleMoves() const override;
  bool makeMove(const GameMove &move) override;
  void undoMove(const GameMove &move) override;
  bool checkWin(const GameMove &lastMove) const override;
  bool isFull() const override;
  int evaluate(int aiPlayer) const override;

  std::string getGameName() const override { return "Gomoku"; }
  int getBoardWidth() const override { return 15; }
  int getBoardHeight() const override { return 15; }
  int getCell(int x, int y) const override;
  int scoreMoveForPlayer(int x, int y, int player) const;
  int estimateMoveLevel(int x, int y, int player) const;
  bool hasUrgentThreat(const GameMove &move) const;
  void reset() override;

private:
  int grid[15][15];
  int moveCount;
  int countDirection(int x, int y, int dx, int dy, int player) const;
  int countDirectionWithMove(int x, int y, int dx, int dy, int player,
                             int moveX, int moveY) const;
  int scoreRun(int count, int openEnds) const;
  int scorePatternAt(int x, int y, int player) const;
  int scorePatternForMove(int x, int y, int player) const;
  int evaluatePlayer(int player) const;
  int levelFromMoveScore(int selectedScore, int bestScore, int rank,
                         int moveCount) const;
};

#endif
