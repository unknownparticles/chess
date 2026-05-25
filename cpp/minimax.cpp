#include "minimax.h"
#include "gomoku_engine.h"

#include <algorithm>
#include <chrono>
#include <climits>
#include <cmath>
#include <cstdlib>
#include <cstdint>
#include <vector>

const int WIN_SCORE = 100000000;
const int TT_EXACT = 0;
const int TT_LOWER = 1;
const int TT_UPPER = 2;
const int TT_SIZE = 1 << 20;

static uint64_t splitMix64(uint64_t value) {
    value += 0x9e3779b97f4a7c15ULL;
    value = (value ^ (value >> 30)) * 0xbf58476d1ce4e5b9ULL;
    value = (value ^ (value >> 27)) * 0x94d049bb133111ebULL;
    return value ^ (value >> 31);
}

MinimaxAI::MinimaxAI(int player) : aiPlayer(player) {
    opponent = (player == 1) ? 2 : 1;
    transpositionEnabled = false;
    searchStartMs = 0;
    timeLimitMs = 0;
    timeExpired = false;
    killerMoves.assign(64, std::vector<GameMove>(2, {-1, -1, 0, 0}));
    historyScore.assign(3, std::vector<int>(225, 0));
    transpositionTable.assign(TT_SIZE, {0, 0, TT_EXACT, {-1, -1, 0, 0}});
}

int MinimaxAI::minimax(GameEngine &engine, int depth, int alpha, int beta,
                       bool isMaximizing, const GameMove &lastMove) {
    if (lastMove.x != -1 && engine.checkWin(lastMove)) {
        return isMaximizing ? -1000000 : 1000000;
    }

    if (depth == 0 || engine.isFull()) {
        return engine.evaluate(aiPlayer);
    }

    std::vector<GameMove> moves = filterSafeRootMoves(engine, engine.getPossibleMoves());

    if (isMaximizing) {
        int maxEval = INT_MIN;
        for (GameMove move : moves) {
            move.player = aiPlayer;
            if (!engine.makeMove(move)) continue;
            int eval = minimax(engine, depth - 1, alpha, beta, false, move);
            engine.undoMove(move);
            maxEval = std::max(maxEval, eval);
            alpha = std::max(alpha, eval);
            if (beta <= alpha) break;
        }
        return maxEval;
    }

    int minEval = INT_MAX;
    for (GameMove move : moves) {
        move.player = opponent;
        if (!engine.makeMove(move)) continue;
        int eval = minimax(engine, depth - 1, alpha, beta, true, move);
        engine.undoMove(move);
        minEval = std::min(minEval, eval);
        beta = std::min(beta, eval);
        if (beta <= alpha) break;
    }
    return minEval;
}

GameMove MinimaxAI::findBestMove(GameEngine &engine, int depth) {
    std::vector<GameMove> moves = filterSafeRootMoves(engine, engine.getPossibleMoves());
    GameMove bestMove = {-1, -1, aiPlayer, INT_MIN};

    for (GameMove move : moves) {
        move.player = aiPlayer;
        if (!engine.makeMove(move)) continue;
        move.score = minimax(engine, depth - 1, INT_MIN, INT_MAX, false, move);
        engine.undoMove(move);
        if (move.score > bestMove.score) {
            bestMove = move;
        }
    }

    return bestMove;
}

MinimaxAI::SearchConfig MinimaxAI::getConfigForLevel(int level) const {
    level = std::max(1, std::min(100, level));
    double t = level / 100.0;

    SearchConfig config;
    config.maxDepth = 1 + static_cast<int>(std::floor(t * 7.0));
    config.moveLimit = std::max(8, 26 - level / 5);
    config.timeLimitMs = 120 + level * 18;
    config.mistakeRate = std::pow((100 - level) / 100.0, 2.0);
    config.useTransposition = level >= 41;
    config.useThreatExtension = level >= 61;

    if (level <= 20) config.maxDepth = 2;
    else if (level <= 40) config.maxDepth = 3;
    else if (level <= 60) config.maxDepth = 5;
    else if (level <= 80) config.maxDepth = 6;

    return config;
}

GameMove MinimaxAI::findBestMoveForLevel(GameEngine &engine, int level) {
    SearchConfig config = getConfigForLevel(level);

    if (dynamic_cast<GomokuEngine *>(&engine) == nullptr) {
        return findBestMove(engine, config.maxDepth);
    }

    return findBestMoveAdvanced(engine, config);
}

GameMove MinimaxAI::findBestMoveAdvanced(GameEngine &engine, const SearchConfig &config) {
    searchStartMs = nowMs();
    timeLimitMs = config.timeLimitMs;
    timeExpired = false;
    transpositionEnabled = config.useTransposition;

    std::fill(transpositionTable.begin(), transpositionTable.end(),
              TTEntry{0, 0, TT_EXACT, {-1, -1, 0, 0}});

    std::vector<GameMove> rootMoves = filterSafeRootMoves(engine, engine.getPossibleMoves());
    if (rootMoves.empty()) return {-1, -1, aiPlayer, INT_MIN};

    GameMove bestMove = rootMoves.front();
    bestMove.player = aiPlayer;
    bestMove.score = INT_MIN;

    for (int depth = 1; depth <= config.maxDepth && !timeExpired; ++depth) {
        std::vector<GameMove> scoredMoves;
        std::vector<GameMove> ordered = orderMoves(engine, rootMoves, aiPlayer, 0, bestMove);
        int searched = 0;
        int alpha = INT_MIN / 2;

        for (GameMove move : ordered) {
            if (searched++ >= config.moveLimit || nowMs() - searchStartMs > timeLimitMs) {
                timeExpired = true;
                break;
            }

            move.player = aiPlayer;
            if (!engine.makeMove(move)) continue;
            int score = -negamax(engine, depth - 1, INT_MIN / 2, INT_MAX / 2,
                                 opponent, move, config.useThreatExtension);
            engine.undoMove(move);

            move.score = score;
            scoredMoves.push_back(move);

            if (!timeExpired && score > bestMove.score) {
                bestMove = move;
            }
            alpha = std::max(alpha, score);
        }

        if (!scoredMoves.empty()) {
            std::sort(scoredMoves.begin(), scoredMoves.end(),
                      [](const GameMove &a, const GameMove &b) {
                          return a.score > b.score;
                      });
            rootMoves = scoredMoves;
            if (depth == config.maxDepth || timeExpired) {
                bestMove = chooseWithMistakeRate(scoredMoves, config.mistakeRate);
            }
        }
    }

    bestMove.player = aiPlayer;
    return bestMove;
}

int MinimaxAI::negamax(GameEngine &engine, int depth, int alpha, int beta, int player,
                       const GameMove &lastMove, bool allowThreatExtension) {
    if (nowMs() - searchStartMs > timeLimitMs) {
        timeExpired = true;
        int value = engine.evaluate(aiPlayer);
        return player == aiPlayer ? value : -value;
    }

    if (lastMove.x != -1 && engine.checkWin(lastMove)) {
        return lastMove.player == player ? WIN_SCORE + depth : -WIN_SCORE - depth;
    }

    if (engine.isFull()) return 0;

    const GomokuEngine *gomoku = dynamic_cast<const GomokuEngine *>(&engine);
    if (depth == 0) {
        if (allowThreatExtension && gomoku != nullptr && gomoku->hasUrgentThreat(lastMove)) {
            depth = 1;
        } else {
            int value = engine.evaluate(aiPlayer);
            return player == aiPlayer ? value : -value;
        }
    }

    int originalAlpha = alpha;
    unsigned long long key = hashBoard(engine, player);
    TTEntry &entry = transpositionTable[key % transpositionTable.size()];
    GameMove ttMove = {-1, -1, player, 0};

    if (transpositionEnabled && entry.depth >= depth && entry.bestMove.x != -1) {
        ttMove = entry.bestMove;
        if (entry.flag == TT_EXACT) return entry.value;
        if (entry.flag == TT_LOWER) alpha = std::max(alpha, entry.value);
        else if (entry.flag == TT_UPPER) beta = std::min(beta, entry.value);
        if (alpha >= beta) return entry.value;
    }

    std::vector<GameMove> moves = orderMoves(engine, engine.getPossibleMoves(), player, depth, ttMove);
    if (moves.empty()) {
        int value = engine.evaluate(aiPlayer);
        return player == aiPlayer ? value : -value;
    }

    int bestValue = INT_MIN / 2;
    GameMove bestMove = {-1, -1, player, 0};
    bool firstMove = true;

    for (GameMove move : moves) {
        move.player = player;
        if (!engine.makeMove(move)) continue;

        int nextPlayer = player == 1 ? 2 : 1;
        int value;
        if (firstMove) {
            value = -negamax(engine, depth - 1, -beta, -alpha, nextPlayer, move,
                             allowThreatExtension);
        } else {
            value = -negamax(engine, depth - 1, -alpha - 1, -alpha, nextPlayer, move,
                             allowThreatExtension);
            if (value > alpha && value < beta) {
                value = -negamax(engine, depth - 1, -beta, -alpha, nextPlayer, move,
                                 allowThreatExtension);
            }
        }

        engine.undoMove(move);
        firstMove = false;

        if (timeExpired) return bestValue;

        if (value > bestValue) {
            bestValue = value;
            bestMove = move;
        }
        alpha = std::max(alpha, value);

        if (alpha >= beta) {
            int ply = std::max(0, std::min(depth, static_cast<int>(killerMoves.size()) - 1));
            if (!sameMove(killerMoves[ply][0], move)) {
                killerMoves[ply][1] = killerMoves[ply][0];
                killerMoves[ply][0] = move;
            }
            historyScore[player][move.y * engine.getBoardWidth() + move.x] += depth * depth;
            break;
        }
    }

    if (transpositionEnabled) {
        entry.depth = depth;
        entry.value = bestValue;
        entry.bestMove = bestMove;
        if (bestValue <= originalAlpha) entry.flag = TT_UPPER;
        else if (bestValue >= beta) entry.flag = TT_LOWER;
        else entry.flag = TT_EXACT;
    }

    return bestValue;
}

std::vector<GameMove> MinimaxAI::orderMoves(GameEngine &engine,
                                            const std::vector<GameMove> &moves,
                                            int player, int ply,
                                            const GameMove &ttMove) const {
    const GomokuEngine *gomoku = dynamic_cast<const GomokuEngine *>(&engine);
    std::vector<GameMove> ordered = moves;
    int width = engine.getBoardWidth();

    for (GameMove &move : ordered) {
        int score = move.score;
        if (sameMove(move, ttMove)) score += WIN_SCORE;

        if (gomoku != nullptr) {
            int rival = player == 1 ? 2 : 1;
            score += gomoku->scoreMoveForPlayer(move.x, move.y, player) * 4;
            score += gomoku->scoreMoveForPlayer(move.x, move.y, rival) * 8;
        }

        int safePly = std::max(0, std::min(ply, static_cast<int>(killerMoves.size()) - 1));
        if (sameMove(move, killerMoves[safePly][0])) score += 500000;
        if (sameMove(move, killerMoves[safePly][1])) score += 250000;

        int idx = move.y * width + move.x;
        if (player >= 0 && player < static_cast<int>(historyScore.size()) &&
            idx >= 0 && idx < static_cast<int>(historyScore[player].size())) {
            score += historyScore[player][idx];
        }

        move.score = score;
    }

    std::sort(ordered.begin(), ordered.end(),
              [](const GameMove &a, const GameMove &b) {
                  return a.score > b.score;
              });
    return ordered;
}

unsigned long long MinimaxAI::hashBoard(const GameEngine &engine, int player) const {
    unsigned long long hash = splitMix64(static_cast<uint64_t>(player));

    for (int y = 0; y < engine.getBoardHeight(); ++y) {
        for (int x = 0; x < engine.getBoardWidth(); ++x) {
            int cell = engine.getCell(x, y);
            if (cell != 0) {
                uint64_t seed = static_cast<uint64_t>(x + 1) * 73856093ULL ^
                                static_cast<uint64_t>(y + 1) * 19349663ULL ^
                                static_cast<uint64_t>(cell) * 83492791ULL;
                hash ^= splitMix64(seed);
            }
        }
    }

    return hash;
}

long long MinimaxAI::nowMs() const {
    using namespace std::chrono;
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

bool MinimaxAI::sameMove(const GameMove &a, const GameMove &b) const {
    return a.x == b.x && a.y == b.y;
}

GameMove MinimaxAI::chooseWithMistakeRate(std::vector<GameMove> moves, double mistakeRate) const {
    if (moves.empty()) return {-1, -1, aiPlayer, INT_MIN};
    if (mistakeRate <= 0.01) return moves.front();

    int pool = std::max(1, std::min(static_cast<int>(moves.size()),
                                    1 + static_cast<int>(mistakeRate * 6)));
    int roll = std::rand() % 10000;
    int index = 0;

    for (int i = 1; i < pool; ++i) {
        int threshold = static_cast<int>(mistakeRate * 10000 / (i + 1));
        if (roll < threshold) index = i;
    }

    return moves[index];
}

std::vector<GameMove> MinimaxAI::immediateWinningMoves(GameEngine &engine, int player) const {
    std::vector<GameMove> wins;
    std::vector<GameMove> moves = engine.getPossibleMoves();

    for (GameMove move : moves) {
        move.player = player;
        if (!engine.makeMove(move)) continue;
        bool win = engine.checkWin(move);
        engine.undoMove(move);
        if (win) wins.push_back(move);
    }

    return wins;
}

int MinimaxAI::opponentThreatRiskAfterMove(GameEngine &engine, GameMove aiMove) const {
    aiMove.player = aiPlayer;
    if (!engine.makeMove(aiMove)) return 1000000000;

    if (engine.checkWin(aiMove)) {
        engine.undoMove(aiMove);
        return -1000000000;
    }

    int worstRisk = 0;
    std::vector<GameMove> opponentMoves = engine.getPossibleMoves();

    for (GameMove oppMove : opponentMoves) {
        oppMove.player = opponent;
        if (!engine.makeMove(oppMove)) continue;

        if (engine.checkWin(oppMove)) {
            worstRisk = std::max(worstRisk, 100000000);
        } else {
            int futureWins = static_cast<int>(immediateWinningMoves(engine, opponent).size());
            if (futureWins >= 2) {
                worstRisk = std::max(worstRisk, 50000000 + futureWins);
            } else if (futureWins == 1) {
                worstRisk = std::max(worstRisk, 1000000);
            }
        }

        engine.undoMove(oppMove);
    }

    engine.undoMove(aiMove);
    return worstRisk;
}

std::vector<GameMove> MinimaxAI::filterSafeRootMoves(GameEngine &engine,
                                                     const std::vector<GameMove> &moves) const {
    if (moves.empty()) return moves;

    std::vector<GameMove> aiWins = immediateWinningMoves(engine, aiPlayer);
    if (!aiWins.empty()) {
        return aiWins;
    }

    std::vector<GameMove> opponentWins = immediateWinningMoves(engine, opponent);
    if (opponentWins.size() == 1) {
        GameMove block = opponentWins.front();
        block.player = aiPlayer;
        return std::vector<GameMove>(1, block);
    }

    int bestRisk = INT_MAX;
    std::vector<GameMove> safest;

    for (GameMove move : moves) {
        move.player = aiPlayer;
        int risk = opponentThreatRiskAfterMove(engine, move);

        if (risk < bestRisk) {
            bestRisk = risk;
            safest.clear();
            safest.push_back(move);
        } else if (risk == bestRisk) {
            safest.push_back(move);
        }
    }

    return safest.empty() ? moves : safest;
}
