#ifndef DECK_H
#define DECK_H

#include <vector>
#include <iostream>
#include <string>
#include "./utils/Logger.h"
#include <map>

class Deck
{
private:
    std::vector<std::string> suits;
    std::vector<std::string> ranks;
    std::vector<std::string> deck;
    std::map<std::string, int> cardToIndex;

    Logger logger;

public:
    Deck();
    ~Deck();
    const std::vector<std::string> &getDeck() const;
    const std::map<std::string, int> &getIndicesMap() const;
};

#endif