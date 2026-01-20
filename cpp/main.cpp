#include "gomoku_engine.h"
#include "minimax.h"
#include "ttt_engine.h"
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

int main() {
  std::unique_ptr<GameEngine> engine =
      std::unique_ptr<GomokuEngine>(new GomokuEngine());
  MinimaxAI ai(2); // AI is player 2
  std::string line;

  while (std::getline(std::cin, line)) {
    if (line == "exit")
      break;
    if (line.empty())
      continue;

    std::stringstream ss(line);
    std::string cmd;
    ss >> cmd;

    if (cmd == "switch") {
      std::string game;
      ss >> game;
      if (game == "gomoku")
        engine = std::unique_ptr<GomokuEngine>(new GomokuEngine());
      else if (game == "ttt")
        engine = std::unique_ptr<TTTEngine>(new TTTEngine());
      std::cout << "switched " << engine->getGameName() << " "
                << engine->getBoardWidth() << " " << engine->getBoardHeight()
                << std::endl;
    } else if (cmd == "move") {
      int x, y, p;
      ss >> x >> y >> p;
      GameMove m = {x, y, p, 0};
      if (engine->makeMove(m)) {
        if (engine->checkWin(m))
          std::cout << "win " << p << std::endl;
        else if (engine->isFull())
          std::cout << "draw" << std::endl;
        else
          std::cout << "ok" << std::endl;
      } else {
        std::cout << "error" << std::endl;
      }
    } else if (cmd == "gen") {
      int depth = 3;
      ss >> depth;
      GameMove best = ai.findBestMove(*engine, depth);
      if (best.x != -1) {
        engine->makeMove(best);
        std::cout << "best " << best.x << " " << best.y << std::endl;
        if (engine->checkWin(best))
          std::cout << "win " << best.player << std::endl;
        else if (engine->isFull())
          std::cout << "draw" << std::endl;
      } else {
        std::cout << "error" << std::endl;
      }
    } else if (cmd == "reset") {
      engine->reset();
      std::cout << "ok" << std::endl;
    }
  }
  return 0;
}
