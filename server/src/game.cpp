#include "game.h"
#include "cards.h"
#include <iostream>
#include <unistd.h>
#include <algorithm>
#include <random>

Game::Game(std::vector<int> players, int start_balance, bool isWinner)
{
    Card::createDeck(this->cards);

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(this->cards.begin(), this->cards.end(), g);

    // std::cout << "Liczba kart w talii: " << this->cards.size() << std::endl;

    // for (Card *card : this->cards)
    // {
    //     std::cout << card->rank << card->suit << " ";
    // }

    // std::cout << std::endl;

    for (int p : players)
    {
        this->player_balance[p] = start_balance;
        std::cout << "player " << p << std::endl;
        std::cout << "Ustawiono balans " << player_balance.at(p) << std::endl;
    }
}

Game::~Game()
{

    for (Card *c : cards)
    {
        delete c;
    }
    cards.clear();
}

void Game::run()
{
    while (!isWinner)
    {
    }
}