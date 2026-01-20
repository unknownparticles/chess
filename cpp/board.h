#ifndef BOARD_H
#define BOARD_H

#include <vector>

const int BOARD_SIZE = 15;
const int EMPTY = 0;
const int BLACK = 1;
const int WHITE = 2;

struct Move {
    int x, y;
    int score;
};

class Board {
public:
    Board();
    bool makeMove(int x, int y, int player);
    void undoMove(int x, int y);
    bool checkWin(int x, int y, int player) const;
    bool isFull() const;
    int getCell(int x, int y) const;
    std::vector<Move> getPossibleMoves() const;

private:
    int grid[BOARD_SIZE][BOARD_SIZE];
    int moveCount;
};

#endif
