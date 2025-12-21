#include "Game.h"
#include "Deck.h"
#include "Board.h"
#include "Server.h"
#include "../omp/HandEvaluator.h"

#include <iostream>
#include <unistd.h>
#include <algorithm>
#include <random>
#include <string.h>
#include <thread>
#include <chrono>

Game::Game(std::vector<Player *> players, int start_balance) : logger()
{
    this->start_balance = start_balance;
    this->isWinner = false;
    this->table_players = players;

    Board gameBoard;

    for (Player *p : players)
    {
        p->setBalance(start_balance);

        logger.log(LogLevel::INFO, "Player ID: " + std::to_string(p->getId()) + " has a balance: " + std::to_string(start_balance));
    }
}

Game::~Game()
{
}

void Game::queueAction(int playerId, const std::string &command, int amount)
{
    std::lock_guard<std::mutex> lock(gameMutex);
    actionQueue.push({playerId, command, amount});
}

void Game::playerBet(Player *p, int amount)
{
    if (p->getBalance() >= amount)
    {
        p->setBalance(p->getBalance() - amount);
    }
    else
    {
        if (server)
        {
            server->sendMessageToPlayer(p->getId(), "You don't have enough balance to bet that amount.\n");
        }
    }
}

void Game::bettingPhase(std::string phaseName)
{
    if (server)
    {
        server->sendMessageToAllPlayers(table_players, "This is " + phaseName + " phase\n");
        server->sendMessageToAllPlayers(table_players, "You have 20 seconds to enter \"BET + amount\" or \"FOLD\" \n");
    };

    for (Player *p : table_players)
    {
        p->setCurrentBet(0);
    }
    auto start = std::chrono::steady_clock::now();
    bool running = true;

    while (running)
    {
        auto end = std::chrono::steady_clock::now();
        auto diff = end - start;
        if (std::chrono::duration_cast<std::chrono::seconds>(end - start).count() >= 20)
        {
            running = false;
            break;
        }

        processQueue();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
};

void Game::processQueue()
{
    std::lock_guard<std::mutex> lock(gameMutex);
    while (actionQueue.size() > 0)
    {
        PlayerAction action = actionQueue.front();

        actionQueue.pop();

        int playerId = action.playerId;
        std::string command = action.command;
        int amount = action.amount;
        if (amount <= 0)
        {
            if (server)
                server->sendMessageToPlayer(playerId, "Invalid chips amount\n");
            continue;
        }
        Player *p = nullptr;

        for (Player *player : table_players)
        {
            if (player->getId() == playerId)
            {
                p = player;
                break;
            }
        }

        if (command == "FOLD")
        {
            p->setStatus(PlayerStatus::FOLDED);
            server->sendMessageToPlayer(playerId, "You folded.\n");
        }

        if (command == "BET")
        {
            if (amount <= p->getCurrentBet())
            {
                p->setCurrentBet(p->getCurrentBet() + amount);
                p->setBalance(p->getBalance() - amount);
                gameBoard.addToPot(amount);

                if (server)
                {
                    server->sendMessageToPlayer(playerId, "Bet accepted: " + std::to_string(amount) + " added to the pot\n");
                    server->sendMessageToAllPlayers(table_players, "Player: " + std::to_string(playerId) + " placed " + std::to_string(amount) + " chips into the pot.\n");
                    server->sendMessageToAllPlayers(table_players, "Pot value: " + std::to_string(gameBoard.getPot()) + "\n");
                }
            }
            else
            {
                if (server)
                {
                    server->sendMessageToPlayer(playerId, "You dont have that much chips\n");
                    server->sendMessageToPlayer(playerId, "Try again.\n");
                }
                continue;
            }
        }
    }
}

void Game::determineWinner()
{
    std::vector<Player *> activePlayers;
    for (Player *p : table_players)
    {
        if (p->getStatus() != PlayerStatus::FOLDED)
        {
            activePlayers.push_back(p);
        }
    }

    if (activePlayers.size() == 1)
    {
        Player *winner = activePlayers[0];
        winner->setBalance(winner->getBalance() + winner->getCurrentBet() * 2);
        if (server)
        {
            server->sendMessageToAllPlayers(table_players, "Player " + std::to_string(winner->getId()) + " wins " + std::to_string(winner->getCurrentBet() * 2) + " chips!\n");
        }
        return;
    }

    int highiestEval = 0;
    Player *highiestEvalPlayer = nullptr;

    for (Player *p : activePlayers)
    {
        omp::HandEvaluator eval;
        omp::Hand h = omp::Hand::empty();
    }

    // omp::HandEvaluator eval;
    // omp::Hand h = omp::Hand::empty();
    // h += omp::Hand(51) + omp::Hand(48) + omp::Hand(0) + omp::Hand(1) + omp::Hand(2); // AdAs2s2h2c
    // std::cout << eval.evaluate(h) << std::endl;                                      // 28684 = 7 * 4096 + 12
}
void Game::run()
{
    while (table_players.size() > 1)
    {
        logger.log(LogLevel::INFO, "Starting a new round.");

        Deck cardDeck;
        this->deck = cardDeck.getDeck();
        this->deckIndices = cardDeck.getIndicesMap();

        gameBoard.resetPot();
        gameBoard.clearBoard();

        for (Player *p : table_players)
        {
            p->resetForNewRound();
        }

        if (server)
        {
            server->sendMessageToAllPlayers(table_players, "New round starting\n");
        }

        for (Player *p : table_players)
        {
            if (p->getBalance() < 10)
            {
                int currBalance = p->getBalance();
                p->setBalance(0);
                gameBoard.addToPot(p->getBalance());
                p->setStatus(PlayerStatus::ALL_IN);
                if (server)
                    server->sendMessageToPlayer(p->getId(), "Paid " + std::to_string(currBalance) + " chips, your current balance is 0\n");
            }
            p->setBalance(p->getBalance() - 10);
            gameBoard.addToPot(10);
            if (server)
                server->sendMessageToPlayer(p->getId(), "Paid entry fee - 10 chips\n");
        }

        for (Player *p : table_players)
        {
            p->setCard1(deck.back());
            deck.pop_back();
            p->setCard2(deck.back());
            deck.pop_back();
            if (server)
            {
                server->sendMessageToPlayer(p->getId(), "Your cards: " + p->getCard1() + " " + p->getCard2() + "\n");
                server->sendMessageToPlayer(p->getId(), "Balance: " + std::to_string(p->getBalance()) + "\n");
            }
        }

        bettingPhase("PRE-FLOP");

        int activeCount = 0;
        for (Player *p : table_players)
        {
            if (p->getStatus() != PlayerStatus::FOLDED)
                activeCount++;
        }
        if (activeCount < 2)
            determineWinner();

        for (int i = 0; i < 3; i++)
        {
            gameBoard.setBoardCard(i, deck.back());
            logger.log(LogLevel::INFO, "Card was revealed: " + deck.back() + "\n");

            if (server)
            {
                server->sendMessageToAllPlayers(table_players, "Card was revealed: " + deck.back() + "\n");
            }
            deck.pop_back();
        }

        bettingPhase("FLOP");

        activeCount = 0;
        for (Player *p : table_players)
        {
            if (p->getStatus() != PlayerStatus::FOLDED)
                activeCount++;
        }
        if (activeCount < 2)
            determineWinner();

        gameBoard.setBoardCard(3, deck.back());
        deck.pop_back();
    }
}