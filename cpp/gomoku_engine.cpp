#include "gomoku_engine.h"
#include <algorithm>
#include <cmath>
#include <functional>

const int EMPTY = 0;
const int BLACK = 1;
const int WHITE = 2;
const int BOARD_SIZE = 15;
const int FIVE_SCORE = 10000000;
const int LIVE_FOUR_SCORE = 1000000;
const int DEAD_FOUR_SCORE = 100000;
const int LIVE_THREE_SCORE = 10000;
const int DEAD_THREE_SCORE = 1000;
const int LIVE_TWO_SCORE = 100;
const int BROKEN_FOUR_SCORE = 250000;
const int BROKEN_THREE_SCORE = 12000;

static int clampLevel(int level) {
  return std::max(1, std::min(100, level));
}

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
  if (move.x < 0 || move.x >= BOARD_SIZE || move.y < 0 ||
      move.y >= BOARD_SIZE)
    return false;
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
        if (near) {
          int attackScore = scoreMoveForPlayer(i, j, BLACK);
          int defenseScore = scoreMoveForPlayer(i, j, WHITE);
          int center = BOARD_SIZE - std::abs(i - BOARD_SIZE / 2) -
                       std::abs(j - BOARD_SIZE / 2);
          moves.push_back({i, j, 0,
                           std::max(attackScore, defenseScore) * 2 +
                               std::min(attackScore, defenseScore) + center});
        }
      }
    }
  }
  std::sort(moves.begin(), moves.end(), [](const GameMove &a,
                                           const GameMove &b) {
    return a.score > b.score;
  });
  return moves;
}

int GomokuEngine::countDirection(int x, int y, int dx, int dy,
                                 int player) const {
  int count = 0;
  int cx = x + dx;
  int cy = y + dy;
  while (cx >= 0 && cx < BOARD_SIZE && cy >= 0 && cy < BOARD_SIZE &&
         grid[cx][cy] == player) {
    count++;
    cx += dx;
    cy += dy;
  }
  return count;
}

int GomokuEngine::countDirectionWithMove(int x, int y, int dx, int dy,
                                         int player, int moveX,
                                         int moveY) const {
  int count = 0;
  int cx = x + dx;
  int cy = y + dy;
  while (cx >= 0 && cx < BOARD_SIZE && cy >= 0 && cy < BOARD_SIZE) {
    int cell = (cx == moveX && cy == moveY) ? player : grid[cx][cy];
    if (cell != player)
      break;
    count++;
    cx += dx;
    cy += dy;
  }
  return count;
}

int GomokuEngine::scoreRun(int count, int openEnds) const {
  if (count >= 5)
    return FIVE_SCORE;
  if (openEnds == 0)
    return 0;
  if (count == 4)
    return openEnds == 2 ? LIVE_FOUR_SCORE : DEAD_FOUR_SCORE;
  if (count == 3)
    return openEnds == 2 ? LIVE_THREE_SCORE : DEAD_THREE_SCORE;
  if (count == 2)
    return openEnds == 2 ? LIVE_TWO_SCORE : LIVE_TWO_SCORE / 4;
  return openEnds == 2 ? 10 : 1;
}

int GomokuEngine::scorePatternAt(int x, int y, int player) const {
  static const int dx[] = {1, 0, 1, 1};
  static const int dy[] = {0, 1, 1, -1};
  int score = 0;
  int liveThreeCount = 0;
  int fourCount = 0;

  for (int i = 0; i < 4; ++i) {
    int left = countDirection(x, y, -dx[i], -dy[i], player);
    int right = countDirection(x, y, dx[i], dy[i], player);
    int count = left + right + 1;
    int openEnds = 0;
    int lx = x - dx[i] * (left + 1);
    int ly = y - dy[i] * (left + 1);
    int rx = x + dx[i] * (right + 1);
    int ry = y + dy[i] * (right + 1);

    if (lx >= 0 && lx < BOARD_SIZE && ly >= 0 && ly < BOARD_SIZE &&
        grid[lx][ly] == EMPTY)
      openEnds++;
    if (rx >= 0 && rx < BOARD_SIZE && ry >= 0 && ry < BOARD_SIZE &&
        grid[rx][ry] == EMPTY)
      openEnds++;

    int lineScore = scoreRun(count, openEnds);
    score += lineScore;
    if (count == 4 && openEnds > 0)
      fourCount++;
    if (count == 3 && openEnds == 2)
      liveThreeCount++;

    for (int offset = -4; offset <= 0; ++offset) {
      bool containsMove = false;
      int playerCount = 0;
      int emptyCount = 0;
      bool blocked = false;

      for (int step = 0; step < 5; ++step) {
        int cx = x + dx[i] * (offset + step);
        int cy = y + dy[i] * (offset + step);
        if (cx == x && cy == y)
          containsMove = true;
        if (cx < 0 || cx >= BOARD_SIZE || cy < 0 || cy >= BOARD_SIZE) {
          blocked = true;
          break;
        }
        if (grid[cx][cy] == player)
          playerCount++;
        else if (grid[cx][cy] == EMPTY)
          emptyCount++;
        else {
          blocked = true;
          break;
        }
      }

      if (!containsMove || blocked)
        continue;

      int beforeX = x + dx[i] * (offset - 1);
      int beforeY = y + dy[i] * (offset - 1);
      int afterX = x + dx[i] * (offset + 5);
      int afterY = y + dy[i] * (offset + 5);
      int windowOpenEnds = 0;
      if (beforeX >= 0 && beforeX < BOARD_SIZE && beforeY >= 0 &&
          beforeY < BOARD_SIZE && grid[beforeX][beforeY] == EMPTY)
        windowOpenEnds++;
      if (afterX >= 0 && afterX < BOARD_SIZE && afterY >= 0 &&
          afterY < BOARD_SIZE && grid[afterX][afterY] == EMPTY)
        windowOpenEnds++;

      if (playerCount == 5) {
        score += FIVE_SCORE;
      } else if (playerCount == 4 && emptyCount == 1) {
        score += windowOpenEnds == 2 ? BROKEN_FOUR_SCORE : DEAD_FOUR_SCORE;
        fourCount++;
      } else if (playerCount == 3 && emptyCount == 2 && windowOpenEnds > 0) {
        score += windowOpenEnds == 2 ? BROKEN_THREE_SCORE : DEAD_THREE_SCORE;
        if (windowOpenEnds == 2)
          liveThreeCount++;
      } else if (playerCount == 2 && emptyCount == 3 && windowOpenEnds == 2) {
        score += LIVE_TWO_SCORE;
      }
    }
  }

  // Double threats usually force the opponent into a narrow defensive path.
  if (fourCount >= 2)
    score += LIVE_FOUR_SCORE;
  if (fourCount >= 1 && liveThreeCount >= 1)
    score += DEAD_FOUR_SCORE;
  if (liveThreeCount >= 2)
    score += LIVE_THREE_SCORE * 3;

  return score;
}

int GomokuEngine::scorePatternForMove(int x, int y, int player) const {
  static const int dx[] = {1, 0, 1, 1};
  static const int dy[] = {0, 1, 1, -1};
  int score = 0;
  int liveThreeCount = 0;
  int fourCount = 0;

  for (int i = 0; i < 4; ++i) {
    int left = countDirectionWithMove(x, y, -dx[i], -dy[i], player, x, y);
    int right = countDirectionWithMove(x, y, dx[i], dy[i], player, x, y);
    int count = left + right + 1;
    int openEnds = 0;
    int lx = x - dx[i] * (left + 1);
    int ly = y - dy[i] * (left + 1);
    int rx = x + dx[i] * (right + 1);
    int ry = y + dy[i] * (right + 1);

    if (lx >= 0 && lx < BOARD_SIZE && ly >= 0 && ly < BOARD_SIZE &&
        grid[lx][ly] == EMPTY)
      openEnds++;
    if (rx >= 0 && rx < BOARD_SIZE && ry >= 0 && ry < BOARD_SIZE &&
        grid[rx][ry] == EMPTY)
      openEnds++;

    int lineScore = scoreRun(count, openEnds);
    score += lineScore;
    if (count == 4 && openEnds > 0)
      fourCount++;
    if (count == 3 && openEnds == 2)
      liveThreeCount++;

    for (int offset = -4; offset <= 0; ++offset) {
      bool containsMove = false;
      int playerCount = 0;
      int emptyCount = 0;
      bool blocked = false;

      for (int step = 0; step < 5; ++step) {
        int cx = x + dx[i] * (offset + step);
        int cy = y + dy[i] * (offset + step);
        if (cx == x && cy == y)
          containsMove = true;
        if (cx < 0 || cx >= BOARD_SIZE || cy < 0 || cy >= BOARD_SIZE) {
          blocked = true;
          break;
        }
        int cell = (cx == x && cy == y) ? player : grid[cx][cy];
        if (cell == player)
          playerCount++;
        else if (cell == EMPTY)
          emptyCount++;
        else {
          blocked = true;
          break;
        }
      }

      if (!containsMove || blocked)
        continue;

      int beforeX = x + dx[i] * (offset - 1);
      int beforeY = y + dy[i] * (offset - 1);
      int afterX = x + dx[i] * (offset + 5);
      int afterY = y + dy[i] * (offset + 5);
      int windowOpenEnds = 0;
      if (beforeX >= 0 && beforeX < BOARD_SIZE && beforeY >= 0 &&
          beforeY < BOARD_SIZE && grid[beforeX][beforeY] == EMPTY)
        windowOpenEnds++;
      if (afterX >= 0 && afterX < BOARD_SIZE && afterY >= 0 &&
          afterY < BOARD_SIZE && grid[afterX][afterY] == EMPTY)
        windowOpenEnds++;

      if (playerCount == 5) {
        score += FIVE_SCORE;
      } else if (playerCount == 4 && emptyCount == 1) {
        score += windowOpenEnds == 2 ? BROKEN_FOUR_SCORE : DEAD_FOUR_SCORE;
        fourCount++;
      } else if (playerCount == 3 && emptyCount == 2 && windowOpenEnds > 0) {
        score += windowOpenEnds == 2 ? BROKEN_THREE_SCORE : DEAD_THREE_SCORE;
        if (windowOpenEnds == 2)
          liveThreeCount++;
      } else if (playerCount == 2 && emptyCount == 3 && windowOpenEnds == 2) {
        score += LIVE_TWO_SCORE;
      }
    }
  }

  if (fourCount >= 2)
    score += LIVE_FOUR_SCORE;
  if (fourCount >= 1 && liveThreeCount >= 1)
    score += DEAD_FOUR_SCORE;
  if (liveThreeCount >= 2)
    score += LIVE_THREE_SCORE * 3;

  return score;
}

int GomokuEngine::scoreMoveForPlayer(int x, int y, int player) const {
  if (x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE ||
      grid[x][y] != EMPTY)
    return 0;
  return scorePatternForMove(x, y, player);
}

int GomokuEngine::estimateMoveLevel(int x, int y, int player) const {
  if (x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE ||
      grid[x][y] != EMPTY)
    return 1;

  int opponent = player == BLACK ? WHITE : BLACK;
  int selectedAttack = scoreMoveForPlayer(x, y, player);
  int selectedDefense = scoreMoveForPlayer(x, y, opponent);
  int selectedScore = selectedAttack * 4 + selectedDefense * 5;
  std::vector<int> candidateScores;

  for (const auto &move : getPossibleMoves()) {
    int attack = scoreMoveForPlayer(move.x, move.y, player);
    int defense = scoreMoveForPlayer(move.x, move.y, opponent);
    candidateScores.push_back(attack * 4 + defense * 5);
  }

  if (candidateScores.empty())
    return 50;

  std::sort(candidateScores.begin(), candidateScores.end(), std::greater<int>());
  int rank = 1;
  for (int score : candidateScores) {
    if (selectedScore >= score)
      break;
    rank++;
  }

  int level = levelFromMoveScore(selectedScore, candidateScores.front(), rank,
                                 moveCount);
  if (selectedAttack >= FIVE_SCORE || selectedDefense >= FIVE_SCORE)
    level = std::max(level, 96);
  else if (selectedAttack >= LIVE_FOUR_SCORE ||
           selectedDefense >= LIVE_FOUR_SCORE)
    level = std::max(level, 90);
  else if (selectedAttack >= DEAD_FOUR_SCORE ||
           selectedDefense >= DEAD_FOUR_SCORE)
    level = std::max(level, 78);
  else if (selectedAttack >= LIVE_THREE_SCORE ||
           selectedDefense >= LIVE_THREE_SCORE)
    level = std::max(level, 62);

  return clampLevel(level);
}

bool GomokuEngine::hasUrgentThreat(const GameMove &move) const {
  return scorePatternAt(move.x, move.y, move.player) >= DEAD_FOUR_SCORE;
}

int GomokuEngine::evaluatePlayer(int player) const {
  int total = 0;

  for (int i = 0; i < BOARD_SIZE; ++i) {
    for (int j = 0; j < BOARD_SIZE; ++j) {
      if (grid[i][j] == player)
        total += scorePatternAt(i, j, player);
      else if (grid[i][j] == EMPTY)
        total += scoreMoveForPlayer(i, j, player) / 4;
    }
  }
  return total;
}

int GomokuEngine::evaluate(int aiPlayer) const {
  int opponent = (aiPlayer == WHITE) ? BLACK : WHITE;
  int aiScore = evaluatePlayer(aiPlayer);
  int opponentScore = evaluatePlayer(opponent);

  // Defense is weighted higher so the AI treats opponent live-four threats as
  // emergencies instead of chasing a smaller attacking pattern.
  return aiScore - opponentScore * 12 / 10;
}

int GomokuEngine::levelFromMoveScore(int selectedScore, int bestScore, int rank,
                                     int moveCount) const {
  if (bestScore <= 0)
    return moveCount <= 2 ? 45 : 30;

  double ratio = static_cast<double>(selectedScore) / bestScore;
  int rankPenalty = std::max(0, rank - 1) * 6;
  int level = static_cast<int>(ratio * 92) + 8 - rankPenalty;

  // Opening moves have many equivalent choices, so avoid over-punishing small
  // positional differences before tactical patterns exist.
  if (moveCount < 4)
    level = std::max(level, 45);

  return clampLevel(level);
}
