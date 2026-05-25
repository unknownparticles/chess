#ifndef MINIMAX_H
#define MINIMAX_H

#include "engine.h"

#include <vector>

class MinimaxAI {
public:
    MinimaxAI(int aiPlayer);
    GameMove findBestMove(GameEngine &engine, int depth);
    GameMove findBestMoveForLevel(GameEngine &engine, int level);

private:
    struct SearchConfig {
        int maxDepth;
        int moveLimit;
        int timeLimitMs;
        double mistakeRate;
        bool useTransposition;
        bool useThreatExtension;
    };

    struct TTEntry {
        int depth;
        int value;
        int flag;
        GameMove bestMove;
    };

    int aiPlayer;
    int opponent;
    std::vector<std::vector<GameMove> > killerMoves;
    std::vector<std::vector<int> > historyScore;
    std::vector<TTEntry> transpositionTable;
    long long searchStartMs;
    long long timeLimitMs;
    bool timeExpired;
    bool transpositionEnabled;

    int minimax(GameEngine &engine, int depth, int alpha, int beta,
                bool isMaximizing, const GameMove &lastMove);
    SearchConfig getConfigForLevel(int level) const;
    GameMove findBestMoveAdvanced(GameEngine &engine, const SearchConfig &config);
    int negamax(GameEngine &engine, int depth, int alpha, int beta, int player,
                const GameMove &lastMove, bool allowThreatExtension);
    std::vector<GameMove> orderMoves(GameEngine &engine, const std::vector<GameMove> &moves,
                                     int player, int ply, const GameMove &ttMove) const;
    unsigned long long hashBoard(const GameEngine &engine, int player) const;
    long long nowMs() const;
    bool sameMove(const GameMove &a, const GameMove &b) const;
    GameMove chooseWithMistakeRate(std::vector<GameMove> moves, double mistakeRate) const;

    std::vector<GameMove> immediateWinningMoves(GameEngine &engine, int player) const;
    int opponentThreatRiskAfterMove(GameEngine &engine, GameMove aiMove) const;
    std::vector<GameMove> filterSafeRootMoves(GameEngine &engine,
                                              const std::vector<GameMove> &moves) const;
};

#endif
