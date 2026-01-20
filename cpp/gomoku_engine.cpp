#include "gomoku_engine.h"
#include <algorithm>

const int EMPTY = 0;
const int BLACK = 1;
const int WHITE = 2;
const int BOARD_SIZE = 15;

GomokuEngine::GomokuEngine() { reset(); }

void GomokuEngine::reset() {
  moveCount = 0;
  for (int i = 0; i < BOARD_SIZE; ++i) {
    for (int j = 0; j < BOARD_SIZE; ++j) {
      grid[i][j] = EMPTY;
    }
  }
}

int GomokuEngine::getCell(int x, int y) const {
  if (x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE)
    return -1;
  return grid[x][y];
}

bool GomokuEngine::makeMove(const GameMove &move) {
  if (grid[move.x][move.y] != EMPTY)
    return false;
  grid[move.x][move.y] = move.player;
  moveCount++;
  return true;
}

void GomokuEngine::undoMove(const GameMove &move) {
  grid[move.x][move.y] = EMPTY;
  moveCount--;
}

bool GomokuEngine::checkWin(const GameMove &lastMove) const {
  int x = lastMove.x;
  int y = lastMove.y;
  int player = lastMove.player;

  static const int dx[] = {1, 0, 1, 1};
  static const int dy[] = {0, 1, 1, -1};

  for (int i = 0; i < 4; ++i) {
    int count = 1;
    for (int step = 1; step < 5; ++step) {
      int nx = x + dx[i] * step;
      int ny = y + dy[i] * step;
      if (nx >= 0 && nx < BOARD_SIZE && ny >= 0 && ny < BOARD_SIZE &&
          grid[nx][ny] == player)
        count++;
      else
        break;
    }
    for (int step = 1; step < 5; ++step) {
      int nx = x - dx[i] * step;
      int ny = y - dy[i] * step;
      if (nx >= 0 && nx < BOARD_SIZE && ny >= 0 && ny < BOARD_SIZE &&
          grid[nx][ny] == player)
        count++;
      else
        break;
    }
    if (count >= 5)
      return true;
  }
  return false;
}

bool GomokuEngine::isFull() const {
  return moveCount == BOARD_SIZE * BOARD_SIZE;
}

std::vector<GameMove> GomokuEngine::getPossibleMoves() const {
  std::vector<GameMove> moves;
  if (moveCount == 0) {
    moves.push_back({BOARD_SIZE / 2, BOARD_SIZE / 2, 0, 0});
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
          moves.push_back({i, j, 0, 0});
      }
    }
  }
  return moves;
}

int GomokuEngine::evaluateLine(int count, int blocks, bool isAI) const {
  if (count >= 5)
    return 100000;
  if (blocks == 2)
    return 0;

  int baseScore = 0;
  if (isAI) {
    switch (count) {
    case 4:
      baseScore = 1000;
      break;
    case 3:
      baseScore = 100;
      break;
    case 2:
      baseScore = 10;
      break;
    }
  } else {
    switch (count) {
    case 4:
      baseScore = 5000;
      break;
    case 3:
      baseScore = 500;
      break;
    case 2:
      baseScore = 50;
      break;
    }
  }
  if (blocks == 1)
    baseScore /= 2;
  return baseScore;
}

int GomokuEngine::evaluate(int aiPlayer) const {
  int opponent = (aiPlayer == WHITE) ? BLACK : WHITE;
  int aiScore = 0;
  int opponentScore = 0;
  static const int dx[] = {1, 0, 1, 1};
  static const int dy[] = {0, 1, 1, -1};

  auto getScore = [&](int player, bool isAI) {
    int total = 0;
    for (int i = 0; i < BOARD_SIZE; ++i) {
      for (int j = 0; j < BOARD_SIZE; ++j) {
        if (grid[i][j] != player)
          continue;
        for (int k = 0; k < 4; ++k) {
          int px = i - dx[k], py = j - dy[k];
          if (px >= 0 && px < BOARD_SIZE && py >= 0 && py < BOARD_SIZE &&
              grid[px][py] == player)
            continue;

          int count = 1, blocks = 0;
          if (px < 0 || px >= BOARD_SIZE || py < 0 || py >= BOARD_SIZE ||
              grid[px][py] != EMPTY)
            blocks++;
          int cx = i, cy = j;
          while (true) {
            cx += dx[k];
            cy += dy[k];
            if (cx >= 0 && cx < BOARD_SIZE && cy >= 0 && cy < BOARD_SIZE &&
                grid[cx][cy] == player)
              count++;
            else
              break;
          }
          if (cx < 0 || cx >= BOARD_SIZE || cy < 0 || cy >= BOARD_SIZE ||
              grid[cx][cy] != EMPTY)
            blocks++;
          total += evaluateLine(count, blocks, isAI);
        }
      }
    }
    return total;
  };

  aiScore = getScore(aiPlayer, true);
  opponentScore = getScore(opponent, false);
  return aiScore - opponentScore;
}
