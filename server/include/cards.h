#ifndef CARD_H
#define CARD_H

#include <vector>
#include <iostream>

struct Card
{
    char rank;
    char suit;

    Card(char r, char s) : rank(r), suit(s) {}

    static void createDeck(std::vector<Card *> &deck)
    {
        char suits[] = {'H', 'D', 'C', 'S'};
        char ranks[] = {'2', '3', '4', '5', '6', '7', '8', '9', 'T', 'J', 'Q', 'K', 'A'};

        for (char s : suits)
        {
            for (char r : ranks)
            {
                deck.push_back(new Card(r, s));
            }
        }
    }
};

#endif