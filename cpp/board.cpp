#include "board.h"

Board::Board() : moveCount(0) {
  for (int i = 0; i < BOARD_SIZE; ++i) {
    for (int j = 0; j < BOARD_SIZE; ++j) {
      grid[i][j] = EMPTY;
    }
  }
}

bool Board::makeMove(int x, int y, int player) {
  if (x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE ||
      grid[x][y] != EMPTY) {
    return false;
  }
  grid[x][y] = player;
  moveCount++;
  return true;
}

void Board::undoMove(int x, int y) {
  if (x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE) {
    grid[x][y] = EMPTY;
    moveCount--;
  }
}

bool Board::checkWin(int x, int y, int player) const {
  static const int dx[] = {1, 0, 1, 1};
  static const int dy[] = {0, 1, 1, -1};

  for (int i = 0; i < 4; ++i) {
    int count = 1;
    // Search in one direction
    for (int step = 1; step < 5; ++step) {
      int nx = x + dx[i] * step;
      int ny = y + dy[i] * step;
      if (nx >= 0 && nx < BOARD_SIZE && ny >= 0 && ny < BOARD_SIZE &&
          grid[nx][ny] == player) {
        count++;
      } else {
        break;
      }
    }
    // Search in the opposite direction
    for (int step = 1; step < 5; ++step) {
      int nx = x - dx[i] * step;
      int ny = y - dy[i] * step;
      if (nx >= 0 && nx < BOARD_SIZE && ny >= 0 && ny < BOARD_SIZE &&
          grid[nx][ny] == player) {
        count++;
      } else {
        break;
      }
    }
    if (count >= 5)
      return true;
  }
  return false;
}

bool Board::isFull() const { return moveCount == BOARD_SIZE * BOARD_SIZE; }

int Board::getCell(int x, int y) const { return grid[x][y]; }

std::vector<Move> Board::getPossibleMoves() const {
  std::vector<Move> moves;
  // Heuristic: only consider moves near existing pieces
  bool hasPieces = (moveCount > 0);
  if (!hasPieces) {
    moves.push_back({BOARD_SIZE / 2, BOARD_SIZE / 2, 0});
    return moves;
  }

  for (int i = 0; i < BOARD_SIZE; ++i) {
    for (int j = 0; j < BOARD_SIZE; ++j) {
      if (grid[i][j] == EMPTY) {
        bool near = false;
        for (int di = -2; di <= 2 && !near; ++di) {
          for (int dj = -2; dj <= 2 && !near; ++dj) {
            int ni = i + di;
            int nj = j + dj;
            if (ni >= 0 && ni < BOARD_SIZE && nj >= 0 && nj < BOARD_SIZE &&
                grid[ni][nj] != EMPTY) {
              near = true;
            }
          }
        }
        if (near)
          moves.push_back({i, j, 0});
      }
    }
  }
  return moves;
}
