#ifndef BOARD_H
#define BOARD_H

#include <string>
#include <vector>
class Board
{
    std::vector<std::string> boardCards;
    int pot;

public:
    Board();
    ~Board();
    std::vector<std::string> getBoardCards() const;
    int getPot() const;

    void addToPot(int amount);
    void setBoardCard(int index, const std::string &card);

    void resetPot();
    void clearBoard();
};

#endif