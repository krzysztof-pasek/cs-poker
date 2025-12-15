poker-project/
├── README.md               # Krótki opis jak skompilować i uruchomić
├── docs/                   # Dokumentacja
│   ├── protocol_spec.txt   # Opis Twojego protokołu (b. ważne!)
│   └── sprawozdanie.pdf    # To co oddasz na koniec
├── server/                 # Kod w C++ (Linux)
│   ├── src/
│   │   ├── main.cpp        # Punkt startowy, obsługa argumentów
│   │   ├── server.cpp      # Obsługa socketów (bind, listen, accept)
│   │   ├── client_handler.cpp # Obsługa pojedynczego klienta
│   │   ├── game_manager.cpp   # Zarządzanie stołami (wątkami)
│   │   └── game_logic.cpp     # Logika pokera, tasowanie, zasady
│   ├── include/            # Pliki nagłówkowe (.h)
│   └── Makefile            # Skrypt kompilacji (wymagany na Linuxie)
└── client/                 # Kod w Pythonie
    ├── main.py             # Główny plik uruchomieniowy
    ├── gui.py              # Klasy odpowiedzialne za Tkinter/PyQt
    ├── network.py          # Wątek sieciowy (odbieranie/wysyłanie)
    └── assets/             # Obrazki kart (png)