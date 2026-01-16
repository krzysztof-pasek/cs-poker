#include "Game.h"
#include "Deck.h"
#include "Board.h"
#include "Server.h"
#include "omp/HandEvaluator.h"

#include <iostream>
#include <unistd.h>
#include <algorithm>
#include <random>
#include <string.h>
#include <thread>
#include <chrono>

Game::Game(std::vector<Player *> players, int start_balance) : server(nullptr)
{
    this->start_balance = start_balance;
    this->table_players = players;

    for (Player *p : players)
    {
        p->setBalance(start_balance);

        logger.log(LogLevel::INFO, "Player ID: " + std::to_string(p->getId()) + " has a balance: " + std::to_string(start_balance));
    }
}

Game::~Game()
{
}

bool Game::isPlayerInGame(int playerId)
{
    std::lock_guard<std::mutex> lock(gameMutex);
    for (Player *p : table_players)
    {
        if (p->getId() == playerId)
            return true;
    }
    return false;
}

void Game::setServer(Server *srv)
{
    this->server = srv;
}

void Game::queueAction(int playerId, const std::string &command, int amount)
{
    std::lock_guard<std::mutex> lock(gameMutex);
    actionQueue.push({playerId, command, amount});
}

void Game::bettingPhase(std::string phaseName)
{
    if (server)
    {
        server->sendMessageToAllPlayers(table_players, "This is " + phaseName + " phase\n");
        std::this_thread::sleep_for(std::chrono::seconds(2));

        server->sendMessageToAllPlayers(table_players, "You have 20 seconds to enter \"BET + amount\" or \"FOLD\" \n");
    };

    {
        std::lock_guard<std::mutex> lock(gameMutex);
        std::queue<PlayerAction> empty;
        std::swap(actionQueue, empty);
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

        logger.log(LogLevel::INFO, "Processing action from Player: " + std::to_string(playerId) + " Command: " + command + " Amount: " + std::to_string(amount));
        if (amount <= 0 && command == "BET")
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

        if (p == nullptr)
        {
            logger.log(LogLevel::WARNING, "Action from unknown/removed player ID: " + std::to_string(playerId));
            continue;
        }

        if (p->getStatus() == PlayerStatus::ALL_IN)
        {
            if (server)
                server->sendMessageToPlayer(playerId, "You are ALL-IN, wait for showdown.\n");
            continue;
        }

        if (p->getStatus() == PlayerStatus::FOLDED)
        {

            if (server)
                server->sendMessageToPlayer(playerId, "You have already folded\n");
            continue;
        }

        if (command == "FOLD")
        {
            p->setStatus(PlayerStatus::FOLDED);
            server->sendMessageToPlayer(playerId, "You folded.\n");
        }

        if (command == "BET")
        {
            if (amount <= p->getBalance())
            {
                p->setCurrentBet(p->getCurrentBet() + amount);
                p->setBalance(p->getBalance() - amount);
                gameBoard.addToPot(amount);

                if (p->getBalance() == 0)
                {
                    p->setStatus(PlayerStatus::ALL_IN);
                    if (server)
                        server->sendMessageToPlayer(playerId, "You are now ALL-IN.\n");
                }

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

uint64_t Game::parseCardToOMP(const std::string &cardStr)
{
    if (cardStr.length() < 2)
        return 0;

    char suitChar = cardStr.back();
    char rankChar = cardStr[0];

    int rank = 0;
    if (rankChar == '1' && cardStr.length() == 3)
    {
        rank = 8;
    }
    else if (isdigit(rankChar))
    {
        rank = (rankChar - '0') - 2;
    }
    else
    {
        if (rankChar == 'T' || rankChar == 't')
            rank = 8;
        if (rankChar == 'J' || rankChar == 'j')
            rank = 9;
        if (rankChar == 'Q' || rankChar == 'q')
            rank = 10;
        if (rankChar == 'K' || rankChar == 'k')
            rank = 11;
        if (rankChar == 'A' || rankChar == 'a')
            rank = 12;
    }

    int suit = 0;
    if (suitChar == 'h' || suitChar == 'H')
        suit = 0;
    if (suitChar == 'd' || suitChar == 'D')
        suit = 1;
    if (suitChar == 'c' || suitChar == 'C')
        suit = 2;
    if (suitChar == 's' || suitChar == 'S')
        suit = 3;

    return 4 * rank + suit;
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
        int totalPot = gameBoard.getPot();
        winner->setBalance(winner->getBalance() + totalPot);
        if (server)
            server->sendMessageToAllPlayers(table_players, "Player " + std::to_string(winner->getId()) + " wins pot: " + std::to_string(totalPot) + "\n");
        return;
    }

    uint64_t highiestEval = 0;
    std::vector<Player *> winners;
    std::vector<std::string> boardCards = gameBoard.getBoardCards();

    static omp::HandEvaluator eval;
    winners.clear();

    for (Player *p : activePlayers)
    {
        if (p == nullptr)
        {
            logger.log(LogLevel::WARNING, "Znaleziono null pointer w activePlayers! Pomijam.\n");
            continue;
        }
        omp::Hand h = omp::Hand::empty();
        h += omp::Hand(parseCardToOMP(p->getCard1()));
        h += omp::Hand(parseCardToOMP(p->getCard2()));

        for (size_t i = 0; i < boardCards.size(); i++)
        {
            h += omp::Hand(parseCardToOMP(boardCards[i]));
        }

        uint64_t val = eval.evaluate(h);

        logger.log(LogLevel::INFO, "Player: " + std::to_string(p->getId()) + " has a hand: " + std::to_string(val) + "\n");

        if (val > highiestEval)
        {
            highiestEval = val;
            winners.clear();
            winners.push_back(p);
        }
        else if (val == highiestEval)
        {
            winners.push_back(p);
        }
    }

    if (server)
    {
        for (Player *p : winners)
        {
            server->sendMessageToAllPlayers(table_players, "Winner by showdown: Player " + std::to_string(p->getId()) + "\n");
        }
    }

    if (!winners.empty())
    {
        for (Player *winner : winners)
        {
            int playerPot = winner->getCurrentBet();
            winner->setBalance(winner->getBalance() + playerPot * 2);
            if (server)
                server->sendMessageToAllPlayers(table_players, "Player " + std::to_string(winner->getId()) + " wins " + std::to_string(playerPot * 2) + " chips!\n");
        }
    }
}

void Game::removeBankruptPlayers()
{
    std::lock_guard<std::mutex> lock(gameMutex);

    auto p = table_players.begin();
    while (p != table_players.end())
    {
        if ((*p)->getBalance() <= 0)
        {
            std::vector<Player *> broadcastList = table_players;

            if (server)
            {
                server->sendMessageToPlayer((*p)->getId(), "You are bankrupt. Goodbye.\n");
                server->sendMessageToAllPlayers(broadcastList, "Player " + std::to_string((*p)->getId()) + " has been eliminated.\n");
            }

            logger.log(LogLevel::INFO, "Player eliminated: " + std::to_string((*p)->getId()));

            delete *p;
            p = table_players.erase(p);
        }
        else
        {
            ++p;
        }
    }
}

void Game::run()
{
    while (table_players.size() > 1)
    {
        logger.log(LogLevel::INFO, "Starting a new round.");
        std::this_thread::sleep_for(std::chrono::seconds(2));

        Deck cardDeck;
        this->deck = cardDeck.getDeck();

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
                gameBoard.addToPot(currBalance);
                p->setStatus(PlayerStatus::ALL_IN);
                if (server)
                    server->sendMessageToPlayer(p->getId(), "Paid " + std::to_string(currBalance) + " chips, your current balance is 0\n");
            }
            else
            {
                p->setBalance(p->getBalance() - 10);
                gameBoard.addToPot(10);
                if (server)
                    server->sendMessageToPlayer(p->getId(), "Paid entry fee - 10 chips\n");
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(2));

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

        std::this_thread::sleep_for(std::chrono::seconds(2));

        bettingPhase("PRE-FLOP");

        int activeCount = 0;
        for (Player *p : table_players)
        {
            if (p->getStatus() != PlayerStatus::FOLDED)
                activeCount++;
        }
        if (activeCount < 2)
        {
            determineWinner();
            continue;
        }

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
        {
            determineWinner();
            continue;
        }

        gameBoard.setBoardCard(3, deck.back());
        logger.log(LogLevel::INFO, "Card was revealed: " + deck.back() + "\n");
        if (server)
        {
            server->sendMessageToAllPlayers(table_players, "Card was revealed: " + deck.back() + "\n");
        }
        deck.pop_back();

        bettingPhase("TURN");

        activeCount = 0;
        for (Player *p : table_players)
        {
            if (p->getStatus() != PlayerStatus::FOLDED)
                activeCount++;
        }
        if (activeCount < 2)
        {
            determineWinner();
            continue;
        }

        gameBoard.setBoardCard(4, deck.back());
        logger.log(LogLevel::INFO, "Card was revealed: " + deck.back() + "\n");
        if (server)
        {
            server->sendMessageToAllPlayers(table_players, "Card was revealed: " + deck.back() + "\n");
        }
        deck.pop_back();

        bettingPhase("RIVER");

        determineWinner();

        removeBankruptPlayers();
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
    if (server)
        server->sendMessageToAllPlayers(table_players, "Game Over! Not enough players.\n");
}