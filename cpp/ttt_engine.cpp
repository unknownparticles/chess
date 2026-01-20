#include "ttt_engine.h"

TTTEngine::TTTEngine() { reset(); }

void TTTEngine::reset() {
  moveCount = 0;
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      grid[i][j] = 0;
    }
  }
}

int TTTEngine::getCell(int x, int y) const {
  if (x < 0 || x >= 3 || y < 0 || y >= 3)
    return -1;
  return grid[x][y];
}

bool TTTEngine::makeMove(const GameMove &move) {
  if (grid[move.x][move.y] != 0)
    return false;
  grid[move.x][move.y] = move.player;
  moveCount++;
  return true;
}

void TTTEngine::undoMove(const GameMove &move) {
  grid[move.x][move.y] = 0;
  moveCount--;
}

bool TTTEngine::checkWin(const GameMove &lastMove) const {
  int p = lastMove.player;
  // Rows and Columns
  for (int i = 0; i < 3; i++) {
    if (grid[i][0] == p && grid[i][1] == p && grid[i][2] == p)
      return true;
    if (grid[0][i] == p && grid[1][i] == p && grid[2][i] == p)
      return true;
  }
  // Diagonals
  if (grid[0][0] == p && grid[1][1] == p && grid[2][2] == p)
    return true;
  if (grid[0][2] == p && grid[1][1] == p && grid[2][0] == p)
    return true;
  return false;
}

bool TTTEngine::isFull() const { return moveCount == 9; }

std::vector<GameMove> TTTEngine::getPossibleMoves() const {
  std::vector<GameMove> moves;
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      if (grid[i][j] == 0)
        moves.push_back({i, j, 0, 0});
    }
  }
  return moves;
}

int TTTEngine::evaluate(int aiPlayer) const {
  // Evaluation for TTT is simple since Minimax will usually hit the end of the
  // tree But we can add a simple heuristic
  return 0;
}
