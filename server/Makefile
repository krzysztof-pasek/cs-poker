CXX = g++

CXXFLAGS = -Iinclude -Wall -std=c++17
LDFLAGS = -pthread


OMP_SRC = include/omp/HandEvaluator.cpp


SRCS_COMMON = src/game.cpp src/server.cpp src/deck.cpp src/player.cpp src/board.cpp $(OMP_SRC)

SRCS_SERVER = src/main.cpp  $(SRCS_COMMON)

SRCS_TEST = src/test.cpp $(SRCS_COMMON)


TARGET_SERVER = server
TARGET_TEST = game_test

all: $(TARGET_SERVER)

$(TARGET_SERVER): $(SRCS_SERVER)
	$(CXX) $(CXXFLAGS) $(SRCS_SERVER) -o $(TARGET_SERVER) $(LDFLAGS)

test: $(SRCS_TEST)
	$(CXX) $(CXXFLAGS) $(SRCS_TEST) -o $(TARGET_TEST) $(LDFLAGS)
	

clean:
	rm -f $(TARGET_SERVER) $(TARGET_TEST)