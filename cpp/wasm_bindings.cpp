#include "gomoku_engine.h"
#include "minimax.h"
#include "ttt_engine.h"
#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <memory>
#include <vector>

using namespace emscripten;

// Wrapper class for game management
class GameWrapper {
private:
  std::unique_ptr<GameEngine> engine;
  std::unique_ptr<MinimaxAI> ai;
  int currentPlayer;
  int aiPlayer;

public:
  GameWrapper(const std::string &gameType) : currentPlayer(1), aiPlayer(2) {
    if (gameType == "gomoku") {
      engine = std::make_unique<GomokuEngine>();
    } else if (gameType == "tictactoe") {
      engine = std::make_unique<TTTEngine>();
    }
    ai = std::make_unique<MinimaxAI>(aiPlayer);
  }

  // Get board dimensions
  int getBoardWidth() const { return engine->getBoardWidth(); }

  int getBoardHeight() const { return engine->getBoardHeight(); }

  // Get cell value at position
  int getCell(int x, int y) const { return engine->getCell(x, y); }

  // Get entire board as a flat array
  val getBoard() const {
    int width = engine->getBoardWidth();
    int height = engine->getBoardHeight();
    val board = val::array();

    for (int i = 0; i < height; i++) {
      for (int j = 0; j < width; j++) {
        board.call<void>("push", engine->getCell(j, i));
      }
    }
    return board;
  }

  // Make a player move
  bool makeMove(int x, int y) {
    GameMove move = {x, y, currentPlayer, 0};
    bool success = engine->makeMove(move);
    if (success) {
      currentPlayer = (currentPlayer == 1) ? 2 : 1;
    }
    return success;
  }

  int estimateMoveLevel(int x, int y, int player) const {
    const GomokuEngine *gomoku = dynamic_cast<const GomokuEngine *>(engine.get());
    if (gomoku != nullptr) {
      return gomoku->estimateMoveLevel(x, y, player);
    }
    return 50;
  }

  // Get AI move. Gomoku treats the argument as a 1-100 level; simpler games
  // still map it back to a search depth inside MinimaxAI.
  val getAIMove(int level) {
    GameMove bestMove = ai->findBestMoveForLevel(*engine, level);
    val result = val::object();
    result.set("x", bestMove.x);
    result.set("y", bestMove.y);
    result.set("score", bestMove.score);
    return result;
  }

  // Make AI move automatically
  bool makeAIMove(int level) {
    GameMove bestMove = ai->findBestMoveForLevel(*engine, level);
    if (bestMove.x == -1 || bestMove.y == -1) {
      return false;
    }
    bestMove.player = aiPlayer;
    bool success = engine->makeMove(bestMove);
    if (success) {
      currentPlayer = (currentPlayer == 1) ? 2 : 1;
    }
    return success;
  }

  // Check if last move won
  bool checkWin(int x, int y, int player) const {
    GameMove move = {x, y, player, 0};
    return engine->checkWin(move);
  }

  // Check if board is full
  bool isFull() const { return engine->isFull(); }

  // Reset game
  void reset() {
    engine->reset();
    currentPlayer = 1;
  }

  // Get current player
  int getCurrentPlayer() const { return currentPlayer; }

  // Get AI player
  int getAIPlayer() const { return aiPlayer; }

  // Set who plays first
  void setFirstPlayer(int player) { currentPlayer = player; }

  // Get game name
  std::string getGameName() const { return engine->getGameName(); }
};

// Emscripten bindings
EMSCRIPTEN_BINDINGS(game_module) {
  class_<GameWrapper>("GameWrapper")
      .constructor<std::string>()
      .function("getBoardWidth", &GameWrapper::getBoardWidth)
      .function("getBoardHeight", &GameWrapper::getBoardHeight)
      .function("getCell", &GameWrapper::getCell)
      .function("getBoard", &GameWrapper::getBoard)
      .function("makeMove", &GameWrapper::makeMove)
      .function("estimateMoveLevel", &GameWrapper::estimateMoveLevel)
      .function("getAIMove", &GameWrapper::getAIMove)
      .function("makeAIMove", &GameWrapper::makeAIMove)
      .function("checkWin", &GameWrapper::checkWin)
      .function("isFull", &GameWrapper::isFull)
      .function("reset", &GameWrapper::reset)
      .function("getCurrentPlayer", &GameWrapper::getCurrentPlayer)
      .function("getAIPlayer", &GameWrapper::getAIPlayer)
      .function("setFirstPlayer", &GameWrapper::setFirstPlayer)
      .function("getGameName", &GameWrapper::getGameName);
}
