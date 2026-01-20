#ifndef ENGINE_H
#define ENGINE_H

#include <string>
#include <vector>

struct GameMove {
  int x, y; // Standard grid coordinates, can be extended for other games
  int player;
  int score;
};

class GameEngine {
public:
  virtual ~GameEngine() {}

  // Core game logic interface
  virtual std::vector<GameMove> getPossibleMoves() const = 0;
  virtual bool makeMove(const GameMove &move) = 0;
  virtual void undoMove(const GameMove &move) = 0;
  virtual bool checkWin(const GameMove &lastMove) const = 0;
  virtual bool isFull() const = 0;
  virtual int evaluate(int aiPlayer) const = 0;

  // UI and state management
  virtual std::string getGameName() const = 0;
  virtual int getBoardWidth() const = 0;
  virtual int getBoardHeight() const = 0;
  virtual int getCell(int x, int y) const = 0;
  virtual void reset() = 0;
};

#endif
