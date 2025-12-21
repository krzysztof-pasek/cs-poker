#include <vector>
#include "Deck.h"
#include <iostream>
#include <algorithm>
#include <random>

Deck::Deck() : logger()
{
    suits = {"C", "D", "H", "S"};
    ranks = {"2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K", "A"};

    for (const auto &suit : suits)
    {
        for (const auto &rank : ranks)
        {
            deck.push_back(rank + suit);
        }
    }

    logger.log(LogLevel::INFO, "Deck initialized with 52 cards.");

    int id = 0;
    for (const auto &card : deck)
    {
        cardToIndex[card] = id++;
    }

    logger.log(LogLevel::INFO, "Deck mapped to indices.");

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(this->deck.begin(), this->deck.end(), g);

    logger.log(LogLevel::INFO, "Deck shuffled.");
}

Deck::~Deck()
{
    logger.log(LogLevel::INFO, "Deck destroyed.");
}

const std::vector<std::string> &Deck::getDeck() const
{
    return deck;
}

const std::map<std::string, int> &Deck::getIndicesMap() const
{
    return cardToIndex;
}