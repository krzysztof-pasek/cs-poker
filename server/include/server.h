#ifndef SERVER_H
#define SERVER_H

#include <vector>
#include <netinet/in.h>
#include <thread>

class Server
{
private:
    int server_socket_fd;
    int port;
    bool is_running;

    std::vector<int> lobby_clients;

    void setup_socket();
    void check_lobby();

public:
    Server(int port_num);
    ~Server();

    void run();
};

#endif