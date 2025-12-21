#include "Player.h"

Player::Player(int id) : id(id), balance(0)
{
}

Player::~Player()
{
}

int Player::getId() const
{
    return id;
}

// Balance operations
int Player::getBalance() const
{
    return balance;
}

void Player::setBalance(int amount)
{
    balance = amount;
}

// Cards operations
const std::string &Player::getCard1() const
{
    return card1;
}

void Player::setCard1(const std::string &card)
{
    card1 = card;
}

const std::string &Player::getCard2() const
{
    return card2;
}

void Player::setCard2(const std::string &card)
{
    card2 = card;
}

// Bet operations

int Player::getCurrentBet() const
{
    return currentBet;
}

void Player::setCurrentBet(int amount)
{
    currentBet = amount;
}

PlayerStatus Player::getStatus() const
{
    return status;
}

void Player::setStatus(PlayerStatus newStatus)
{
    status = newStatus;
}

void Player::resetForNewRound()
{
    card1.clear();
    card2.clear();
    currentBet = 0;
    status = PlayerStatus::ACTIVE;
}