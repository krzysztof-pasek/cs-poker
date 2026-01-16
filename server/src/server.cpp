#include "Server.h"
#include "Player.h"
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "Game.h"
#include <sstream>
#include <thread>
#include <string.h>
#include <algorithm>
#include <vector>
#include <signal.h>

Server::Server(int port_num) : port(port_num), is_running(false), logger()
{
	signal(SIGPIPE, SIG_IGN);
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
		close(server_socket_fd);
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
	std::cout << "Server listening on port " << port << "..." << std::endl;

	while (is_running)
	{
		sockaddr_in client_addr;
		socklen_t client_len = sizeof(client_addr);
		int client_socket = accept(server_socket_fd, (struct sockaddr *)&client_addr, &client_len);

		if (client_socket < 0)
			continue;

		std::cout << "New connection: " << client_socket << std::endl;

		std::thread clientThread(&Server::clientHandler, this, client_socket);
		clientThread.detach();
	}
}

void Server::clientHandler(int client_socket)
{
	{
		std::lock_guard<std::mutex> lock(lobbyMutex);
		lobby_clients.push_back(client_socket);
	}
	check_lobby();

	char buffer[1024];
	while (true)
	{
		memset(buffer, 0, 1024);

		int bytes_read = recv(client_socket, buffer, 1024, 0);
		if (bytes_read <= 0)
		{
			close(client_socket);
			break;
		}

		std::string msg(buffer);
		handleClientMessage(client_socket, msg);
	}
}

void Server::check_lobby()
{
	std::lock_guard<std::mutex> lock(lobbyMutex);

	while (lobby_clients.size() >= 3)
	{
		logger.log(LogLevel::INFO, "Creating new game with 3 players.");
		std::vector<Player *> players_for_game;

		for (int i = 0; i < 3; i++)
		{
			Player *player = new Player(lobby_clients[0]);
			players_for_game.push_back(player);
			lobby_clients.erase(lobby_clients.begin());
		}

		Game *newGame = new Game(players_for_game, 1000);
		newGame->setServer(this);

		{
			std::lock_guard<std::mutex> lock(gamesMutex);
			activeGames.push_back(newGame);
		}

		std::thread gameThread([this, newGame]()
							   {
            newGame->run();
            
            {
                std::lock_guard<std::mutex> lock(this->gamesMutex);
                auto it = std::find(this->activeGames.begin(), this->activeGames.end(), newGame);
                if (it != this->activeGames.end()) {
                    this->activeGames.erase(it);
                }
            }
            
            delete newGame;
            std::cout << "Game object deleted safely." << std::endl; });

		gameThread.detach();
		logger.log(LogLevel::INFO, "Game started with 3 players.");
	}
}

void Server::sendMessageToPlayer(int player_id, const std::string &message)
{
	send(player_id, message.c_str(), message.length(), 0);
}

void Server::sendMessageToAllPlayers(const std::vector<Player *> &players, const std::string &message)
{
	for (Player *p : players)
	{
		sendMessageToPlayer(p->getId(), message);
	}
}

void Server::handleClientMessage(int playerId, std::string message)
{
	std::stringstream ss(message);
	std::string command;
	int amount = 0;
	ss >> command;

	if (command == "BET")
	{
		ss >> amount;
	}

	std::lock_guard<std::mutex> lock(gamesMutex);

	bool gameFound = false;

	for (Game *game : activeGames)
	{
		if (game->isPlayerInGame(playerId))
		{
			game->queueAction(playerId, command, amount);
			gameFound = true;
			break;
		}
	}

	if (!gameFound)
	{
		logger.log(LogLevel::WARNING, "Player " + std::to_string(playerId) + " sent message but is not in any active game.");
	}
}

void Server::stop()
{
	is_running = false;
	close(server_socket_fd);
}