#ifndef MINIMAX_H
#define MINIMAX_H

#include "engine.h"

class MinimaxAI {
public:
  MinimaxAI(int aiPlayer);
  GameMove findBestMove(GameEngine &engine, int depth);

private:
  int aiPlayer;
  int opponent;
  int minimax(GameEngine &engine, int depth, int alpha, int beta,
              bool isMaximizing, const GameMove &lastMove);
};

#endif
