#include "Server.h"
#include <iostream>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cout << "Użycie: ./poker_server <port>" << std::endl;
        return 1;
    }

    int port = std::stoi(argv[1]);

    Server poker_server(port);

    poker_server.run();

    return 0;
}