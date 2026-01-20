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
  void reset() override;

private:
  int grid[15][15];
  int moveCount;
  int evaluateLine(int count, int blocks, bool isAI) const;
};

#endif
