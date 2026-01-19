# Real-Time Multiplayer Poker

![Server](https://img.shields.io/badge/Server-C++-blue)
![Client](https://img.shields.io/badge/Client-Python-yellow)

## 📖 Overview

This project is a real-time multiplayer Texas Hold'em Poker game implementation for Linux operating system. It features a multi-threaded server written in C++ and a user-friendly client application written in Python.

The system relies on TCP sockets for reliable communication, ensuring game state synchronization between all connected players.

### 🏗 Architecture

* Server (C++): Handles game logic, deck management, connection pooling, and state broadcasting.
* Client (Python): Handles graphical user interface and listens for server updates to render the game state.

---

### Prerequisites

* **C++ Compiler:** GCC (g++) or Clang supporting C++17 or later
* **Python:** Version 3.8 or higher
* **Make** 


### 📂 Directory Structure

```text
cs-poker/
├── server/          
│   ├── src/
│   ├── include/
│   └── Makefile
├── client/          
│   ├── gui.py
│   ├── network.py
│   └── main.py
├── .gitignore
└── README.md
```

## 🛠 Installation & Usage

### 1. Setting up the Server
```
cd server

# compilation
make

# starting a server
./server [PORT]
```

### 2. Setting up the Client
```
cd client

# starting a client 
python3 main.py
```
