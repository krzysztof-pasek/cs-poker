Marcin Gawlak 160502  
Krzysztof Pasek 159399

---

**Temat:** Implementacja gry wieloosobowej Poker (Texas Hold'em) w architekturze klient-serwer.

---

## 1. Opis projektu

**Krótki opis:** Celem projektu było stworzenie sieciowej gry w Pokera (wariant Texas Hold'em), umożliwiającej jednoczesną rozgrywkę wielu użytkownikom podłączonym do centralnego serwera. System został zaprojektowany w architekturze klient-serwer, gdzie serwer pełni rolę „krupiera” – zarządza stanem gry, talią kart, licytacją oraz weryfikacją zwycięzcy, natomiast aplikacja kliencka odpowiada za interfejs użytkownika i przesyłanie decyzji gracza.

Logika gry obsługuje standardowe fazy rozgrywki: **Pre-flop** (rozdanie kart graczom), **Flop** (odkrycie 3 kart na stole), **Turn** (odkrycie 4. karty), **River** (odkrycie 5. karty) oraz **Showdown** (określenie zwycięzcy). Serwer dba o synchronizację stanu stołu u wszystkich podłączonych graczy, uniemożliwiając wykonywanie ruchów poza kolejnością czy podglądanie kart przeciwników. Każda faza licytacji trwa 20 sekund, podczas których gracze mogą postawić stawkę (BET) lub spasować (FOLD). Serwer automatycznie tworzy nowe gry, gdy zbierze się 3 graczy w lobby.

**Użyta technologia:**

- **Język programowania:** C++ (serwer) oraz Python (klient).

- **Komunikacja sieciowa:** System gniazd BSD (`sys/socket.h`, `arpa/inet.h`) wykorzystujący protokół TCP – wybrano TCP ze względu na konieczność gwarancji dostarczenia pakietów i zachowania ich kolejności, co jest kluczowe w grach turowych.

- **Interfejs graficzny (Klient):** Tkinter – biblioteka GUI wbudowana w Pythona, umożliwiająca tworzenie okienkowych aplikacji. Interfejs wyświetla karty gracza, karty na stole, saldo, wartość puli oraz przyciski do wykonywania akcji.

- **Wielowątkowość:** Wykorzystanie `std::thread` (C++) oraz `threading` (Python) do obsługi wielu klientów jednocześnie przez serwer. Każdy klient obsługiwany jest w osobnym wątku, a każda gra działa w osobnym wątku wykonawczym.

- **Zewnętrzna biblioteka:** Biblioteka **OMP (Omaha Poker Machine)** do profesjonalnej oceny rąk pokerowych. Biblioteka ta, umieszczona w katalogu `server/include/omp/`, zawiera klasę `HandEvaluator`, która umożliwia precyzyjne określenie siły ręki pokerowej poprzez porównanie wartości numerycznych rąk wszystkich graczy w fazie Showdown.

---

## 2. Opis komunikacji pomiędzy serwerem i klientem

Komunikacja odbywa się w trybie asynchronicznym przy użyciu gniazd sieciowych (Sockets TCP). Wymiana danych oparta jest na przesyłaniu tekstowych wiadomości, które zawierają polecenie oraz ewentualne parametry oddzielone spacjami. Każda wiadomość kończy się znakiem nowej linii (`\n`).

**Schemat protokołu:** Komunikacja wykorzystuje prosty protokół tekstowy. Wiadomości od klienta do serwera mają format: `[KOMENDA] [PARAMETRY]`, np. `"BET 100"` lub `"FOLD"`. Wiadomości od serwera do klienta są bardziej opisowe i zawierają pełne informacje o stanie gry, np. `"Your cards: KH AS"` lub `"Player 1 wins pot: 500"`.

**Przebieg komunikacji:**

1. **Nawiązanie połączenia:**
   - Klient → `connect((server_address, port))` → Serwer
   - Serwer akceptuje połączenie w metodzie `accept()` i tworzy nową instancję wątku `clientHandler` dla tego gracza, przypisując mu identyfikator równy deskryptorowi gniazda.

2. **Dołączenie do gry:**
   - Gracz dodawany jest do `lobby_clients`. Serwer wysyła: `"You are Player <id>\n"`.

3. **Automatyczne tworzenie gry:**
   - Gdy zbierze się 3 graczy w lobby, serwer tworzy nową instancję klasy `Game`, przekazuje im początkową liczbę żetonów (1000) i uruchamia grę w osobnym wątku.

4. **Rozgrywka (Faza licytacji):**
   - Serwer wysyła sygnał: `"This is PRE-FLOP phase\n"` oraz `"You have 20 seconds to enter \"BET + amount\" or \"FOLD\"\n"`.
   - Klient wykonuje ruch i wysyła: `"BET 100\n"` lub `"FOLD\n"`.
   - Serwer przetwarza akcję w kolejce (`actionQueue`), waliduje ruch (sprawdza saldo, status gracza), aktualizuje pulę i rozsyła do wszystkich graczy: `"Player: 1 placed 100 chips into the pot.\n"` oraz `"Pot value: 330\n"`.

5. **Aktualizacja stanu gry:**
   - Po każdej akcji serwer rozsyła aktualizacje stanu: `"Balance: 900\n"`, `"Card was revealed: KH\n"` (po odkryciu karty na stole), `"Your cards: AS KD\n"` (przy rozdaniu).

6. **Określenie zwycięzcy:**
   - W fazie Showdown serwer wykorzystuje bibliotekę OMP do obliczenia wartości rąk wszystkich aktywnych graczy i rozsyła: `"Winner by showdown: Player 1\n"` oraz `"Player 1 wins <kwota> chips!\n"`.

Dzięki zastosowaniu strumieni TCP, dane są na bieżąco odczytywane przez klienta w osobnym wątku (`_receive_messages`) i buforowane w kolejce (`message_queue`), co umożliwia asynchroniczne przetwarzanie wiadomości bez blokowania głównego wątku interfejsu graficznego. Klient wykorzystuje bufor do przechowywania niepełnych wiadomości i łączenia fragmentów przed parsowaniem.

---

## 3. Podsumowanie

**Najważniejsze informacje o implementacji:**

Projekt został podzielony na dwa główne moduły:

- **Serwer (C++):** Aplikacja konsolowa nasłuchująca na określonym porcie (domyślnie 8080). Kluczowym elementem jest klasa `Game`, która przechowuje stan gry (gracze, karty, pula) i pilnuje, aby gracze nie mogli wykonać nielegalnych ruchów (np. podbicia, gdy nie mają żetonów). Serwer wykorzystuje klasę `Server` do zarządzania połączeniami, klasę `Deck` do generowania i tasowania kart, klasę `Board` do przechowywania kart na stole oraz klasę `Player` do reprezentacji każdego gracza. Każda gra działa w osobnym wątku, co umożliwia równoległe prowadzenie wielu stołów.

- **Klient (Python):** Aplikacja okienkowa wykorzystująca Tkinter. Głównym wyzwaniem była tutaj obsługa zdarzeń sieciowych bez blokowania głównego wątku UI. Rozwiązano to poprzez wykorzystanie osobnego wątku do odbierania wiadomości z serwera, buforowanie ich w kolejce (`queue.Queue`) oraz okresowe sprawdzanie kolejki w głównym wątku GUI poprzez `root.after(100, self.process_messages)`, co umożliwia aktualizację widoku na podstawie danych z serwera. Klient parsuje wiadomości z serwera używając wyrażeń regularnych (`re`), co pozwala wyodrębnić informacje o kartach, saldzie, puli i zwycięzcach.

- **Zewnętrzna biblioteka OMP:** Do oceny rąk pokerowych wykorzystano zewnętrzną bibliotekę OMP (Omaha Poker Machine), umieszczoną w katalogu `server/include/omp/`. Biblioteka ta zawiera klasę `HandEvaluator`, która umożliwia precyzyjne określenie siły ręki pokerowej poprzez konwersję kart z formatu tekstowego na format wewnętrzny biblioteki i obliczenie wartości numerycznej ręki. Wyższa wartość oznacza lepszą rękę, co pozwala na jednoznaczne określenie zwycięzcy w fazie Showdown.

**Co sprawiło trudność:**

- **Synchronizacja wątków:** Największym wyzwaniem była obsługa wielu klientów jednocześnie oraz równoległe prowadzenie wielu gier. Konieczne było zabezpieczenie zasobów gry (np. puli żetonów, listy graczy) przed dostępem z wielu wątków w tym samym czasie. Rozwiązano to poprzez użycie mutexów (`std::mutex`): `lobbyMutex` chroni listę klientów oczekujących na grę, `gamesMutex` chroni listę aktywnych gier, a każda gra ma własny `gameMutex` chroniący jej stan. Dodatkowo, akcje graczy są kolejkowane w `actionQueue` i przetwarzane sekwencyjnie w metodzie `processQueue()`, co eliminuje ryzyko konfliktów.

- **Parsowanie wiadomości:** Problematyczna była obsługa sytuacji, w której wiadomość TCP została podzielona na fragmenty lub sklejona z inną (fragmentacja strumienia). Rozwiązano to poprzez buforowanie odebranych danych i przetwarzanie tylko pełnych wiadomości zakończonych znakiem nowej linii (`\n`). Klient wykorzystuje bufor `buffer` do przechowywania niepełnych wiadomości i łączenia fragmentów przed wyodrębnieniem pełnych linii.

- **Obsługa rozłączeń:** Trudnym aspektem było prawidłowe obsługiwanie sytuacji, gdy gracz rozłączy się podczas gry. Serwer musi wykryć rozłączenie (przez `recv()` zwracające 0 lub błąd), usunąć gracza z listy, powiadomić pozostałych graczy oraz w razie potrzeby zakończyć grę, gdy zostanie mniej niż 2 graczy.

Projekt zrealizowano w sposób umożliwiający stabilną rozgrywkę wielu graczy jednocześnie, z pełną synchronizacją stanu i profesjonalną oceną rąk dzięki wykorzystaniu zewnętrznej biblioteki OMP.
