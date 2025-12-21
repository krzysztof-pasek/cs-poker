#ifndef SERVER_H
#define SERVER_H

#include <vector>
#include <string>
#include <netinet/in.h>
#include <mutex>
#include "./utils/Logger.h"

class Game;
class Player;

class Server
{
private:
    int server_socket_fd;
    int port;
    bool is_running;
    std::vector<int> lobby_clients;
    std::mutex lobbyMutex;
    Logger logger;

    Game *activeGame;
    std::mutex gamePtrMutex;

    void setup_socket();
    void clientHandler(int client_socket);
    void check_lobby();

public:
    Server(int port_num);
    ~Server();
    void run();
    void stop();
    void sendMessageToPlayer(int player_id, const std::string &message);
    void sendMessageToAllPlayers(const std::vector<Player *> &players, const std::string &message);
    void handleClientMessage(int playerId, std::string message);
};

#endif