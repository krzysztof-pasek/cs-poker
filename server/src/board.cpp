#include "Board.h"

Board::Board() : pot(0)
{
}

Board::~Board()
{
}

std::vector<std::string> Board::getBoardCards() const
{
    return boardCards;
}

int Board::getPot() const
{
    return pot;
}

void Board::addToPot(int amount)
{
    pot += amount;
}

void Board::setBoardCard(int index, const std::string &card)
{
    if (index >= 0 && index < 5)
    {
        boardCards[index] = card;
    }
}

void Board::resetPot()
{
    pot = 0;
}

void Board::clearBoard()
{
    for (auto &card : boardCards)
    {
        card = "";
    }
}