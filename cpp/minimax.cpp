#include "minimax.h"
#include <algorithm>
#include <climits>

MinimaxAI::MinimaxAI(int player) : aiPlayer(player) {
  opponent = (player == 1) ? 2 : 1; // Assuming 1 and 2 for players
}

int MinimaxAI::minimax(GameEngine &engine, int depth, int alpha, int beta,
                       bool isMaximizing, const GameMove &lastMove) {
  // Check win condition for the PREVIOUS move
  // Note: checkWin(lastMove) checks if the last player won
  if (lastMove.x != -1 && engine.checkWin(lastMove)) {
    return isMaximizing ? -1000000 : 1000000;
  }

  if (depth == 0 || engine.isFull()) {
    return engine.evaluate(aiPlayer);
  }

  std::vector<GameMove> moves = engine.getPossibleMoves();
  if (isMaximizing) {
    int maxEval = INT_MIN;
    for (const auto &move : moves) {
      GameMove m = move;
      m.player = aiPlayer;
      engine.makeMove(m);
      int eval = minimax(engine, depth - 1, alpha, beta, false, m);
      engine.undoMove(m);
      maxEval = std::max(maxEval, eval);
      alpha = std::max(alpha, eval);
      if (beta <= alpha)
        break;
    }
    return maxEval;
  } else {
    int minEval = INT_MAX;
    for (const auto &move : moves) {
      GameMove m = move;
      m.player = opponent;
      engine.makeMove(m);
      int eval = minimax(engine, depth - 1, alpha, beta, true, m);
      engine.undoMove(m);
      minEval = std::min(minEval, eval);
      beta = std::min(beta, eval);
      if (beta <= alpha)
        break;
    }
    return minEval;
  }
}

GameMove MinimaxAI::findBestMove(GameEngine &engine, int depth) {
  std::vector<GameMove> moves = engine.getPossibleMoves();
  GameMove bestMove = {-1, -1, aiPlayer, INT_MIN};

  for (auto &move : moves) {
    move.player = aiPlayer;
    engine.makeMove(move);
    move.score = minimax(engine, depth - 1, INT_MIN, INT_MAX, false, move);
    engine.undoMove(move);
    if (move.score > bestMove.score) {
      bestMove = move;
    }
  }
  return bestMove;
}
