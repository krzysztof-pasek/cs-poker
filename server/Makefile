CXX = g++
CXXFLAGS = -Iinclude -Wall -std=c++17
LDFLAGS = -pthread

# 1. Pliki wspólne (logika gry, której używa i serwer, i testy)
# Zakładam, że masz cards.cpp. Jeśli implementacja kart jest w .h, usuń src/cards.cpp z tej linii.
SRCS_COMMON = src/game.cpp

# 2. Pliki tylko dla serwera (tu jest main serwera)
SRCS_SERVER = src/main.cpp src/server.cpp $(SRCS_COMMON)

# 3. Pliki tylko dla testów (tu jest main testowy)
SRCS_TEST = src/test.cpp $(SRCS_COMMON)

# Nazwy plików wynikowych
TARGET_SERVER = server
TARGET_TEST = game_test

# Domyślna komenda (buduje tylko serwer)
all: $(TARGET_SERVER)

# Reguła budowania SERWERA
$(TARGET_SERVER): $(SRCS_SERVER)
	$(CXX) $(CXXFLAGS) $(SRCS_SERVER) -o $(TARGET_SERVER) $(LDFLAGS)

# Reguła budowania TESTÓW (wpisz 'make test')
test: $(SRCS_TEST)
	$(CXX) $(CXXFLAGS) $(SRCS_TEST) -o $(TARGET_TEST) $(LDFLAGS)
	@echo "Zbudowano testy! Uruchom wpisujac: ./$(TARGET_TEST)"

# Sprzątanie (usuwa oba programy)
clean:
	rm -f $(TARGET_SERVER) $(TARGET_TEST)