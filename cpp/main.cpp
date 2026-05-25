#include "gomoku_engine.h"
#include "minimax.h"
#include "ttt_engine.h"

#include <iostream>
#include <memory>
#include <sstream>
#include <string>

static void playBestMove(GameEngine &engine, MinimaxAI &ai, const GameMove &best) {
    if (best.x == -1 || best.y == -1) {
        std::cout << "error" << std::endl;
        return;
    }

    if (!engine.makeMove(best)) {
        std::cout << "error" << std::endl;
        return;
    }

    std::cout << "best " << best.x << " " << best.y << std::endl;

    if (engine.checkWin(best)) {
        std::cout << "win " << best.player << std::endl;
    } else if (engine.isFull()) {
        std::cout << "draw" << std::endl;
    }
}

int main() {
    std::unique_ptr<GameEngine> engine = std::unique_ptr<GameEngine>(new GomokuEngine());
    MinimaxAI ai(2); // AI is player 2.

    std::string line;
    while (std::getline(std::cin, line)) {
        if (line == "exit") break;
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string cmd;
        ss >> cmd;

        if (cmd == "switch") {
            std::string game;
            ss >> game;

            if (game == "gomoku") {
                engine = std::unique_ptr<GameEngine>(new GomokuEngine());
            } else if (game == "ttt") {
                engine = std::unique_ptr<GameEngine>(new TTTEngine());
            }

            std::cout << "switched " << engine->getGameName() << " "
                      << engine->getBoardWidth() << " "
                      << engine->getBoardHeight() << std::endl;
        } else if (cmd == "move") {
            int x, y, p;
            ss >> x >> y >> p;
            GameMove m = {x, y, p, 0};

            if (engine->makeMove(m)) {
                if (engine->checkWin(m)) {
                    std::cout << "win " << p << std::endl;
                } else if (engine->isFull()) {
                    std::cout << "draw" << std::endl;
                } else {
                    std::cout << "ok" << std::endl;
                }
            } else {
                std::cout << "error" << std::endl;
            }
        } else if (cmd == "gen") {
            int depth = 3;
            ss >> depth;
            GameMove best = ai.findBestMove(*engine, depth);
            playBestMove(*engine, ai, best);
        } else if (cmd == "level") {
            int level = 100;
            ss >> level;
            GameMove best = ai.findBestMoveForLevel(*engine, level);
            playBestMove(*engine, ai, best);
        } else if (cmd == "reset") {
            engine->reset();
            std::cout << "ok" << std::endl;
        }
    }

    return 0;
}
