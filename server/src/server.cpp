#include "server.h"
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

Server::Server(int port_num) : port(port_num), is_running(false)
{
	server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket_fd == -1)
	{
		perror("Socket creation failed");
		exit(1);
	}
	setup_socket();
}

Server::~Server()
{
	if (server_socket_fd != -1)
	{
		close(server_socket_fd);
		std::cout << "Serwer zamknięty, socket zwolniony." << std::endl;
	}
}

void Server::setup_socket()
{
	sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(port);

	int opt = 1;
	setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	if (bind(server_socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		perror("Bind failed");
		exit(1);
	}

	if (listen(server_socket_fd, 10) < 0)
	{
		perror("Listen failed");
		exit(1);
	}
}

void Server::run()
{
	is_running = true;
	std::cout << "Serwer nasłuchuje na porcie " << port << "..." << std::endl;

	while (is_running)
	{
		sockaddr_in client_addr;
		socklen_t client_len = sizeof(client_addr);

		int client_socket = accept(server_socket_fd, (struct sockaddr *)&client_addr, &client_len);

		send(client_socket, "Witaj na serwerze gry!\n", 26, 0);

		if (client_socket < 0)
		{
			perror("Accept failed");
			continue;
		}

		std::cout << "Nowy gracz dołączył! Socket ID: " << client_socket << std::endl;

		lobby_clients.push_back(client_socket);

		check_lobby();
	}
}

void Server::check_lobby()
{

	if (lobby_clients.size() >= 3)
	{
		std::cout << "Mamy komplet! Tworzenie nowego stołu..." << std::endl;

		std::vector<int> players_for_game;
		for (int i = 0; i < 3; i++)
		{
			players_for_game.push_back(lobby_clients[0]);
			lobby_clients.erase(lobby_clients.begin());
		}
	}
}