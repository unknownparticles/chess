#ifndef TTT_ENGINE_H
#define TTT_ENGINE_H

#include "engine.h"

class TTTEngine : public GameEngine {
public:
  TTTEngine();
  std::vector<GameMove> getPossibleMoves() const override;
  bool makeMove(const GameMove &move) override;
  void undoMove(const GameMove &move) override;
  bool checkWin(const GameMove &lastMove) const override;
  bool isFull() const override;
  int evaluate(int aiPlayer) const override;

  std::string getGameName() const override { return "Tic-Tac-Toe"; }
  int getBoardWidth() const override { return 3; }
  int getBoardHeight() const override { return 3; }
  int getCell(int x, int y) const override;
  void reset() override;

private:
  int grid[3][3];
  int moveCount;
};

#endif
