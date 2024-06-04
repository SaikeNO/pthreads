# Problem Śpiącego Barbera

## Opis Projektu
Projekt implementuje klasyczny problem synchronizacji znany jako "Problem Śpiącego Barbera" w dwóch wersjach:

### Wersja z semaforami i mutexami (`sem_only`)

Ta wersja problemu śpiącego barbera używa semaforów do zarządzania dostępem do zasobów oraz mutexów do zapewnienia wzajemnego wykluczania.

### Wersja ze zmiennymi warunkowymi i kolejką (`conditional`)

Ta wersja problemu śpiącego barbera używa zmiennych warunkowych oraz kolejki do synchronizacji procesów i zarządzania kolejnością klientów.


## Struktura Projektu
- `sem_only/`: Wersja implementacji z użyciem semaforów i mutexów.
  - `main.c`: Główny plik źródłowy zawierający implementację.
  - `makefile`: Plik makefile do kompilacji projektu.

- `conditional/`: Wersja implementacji z użyciem zmiennych warunkowych i kolejki.
  - `headers/queue.h`: Plik nagłówkowy definiujący kolejkę.
  - `src/main.c`: Główny plik źródłowy zawierający implementację.
  - `src/queue.c`: Plik źródłowy zawierający implementację kolejki.
  - `makefile`: Plik makefile do kompilacji projektu.

## Kompilacja i Uruchomienie
Aby skompilować i uruchomić dowolną wersję projektu, wykonaj następujące kroki:

### Skompiluj projekt:
```sh
make
```
#### Przykładowe użycie:
```sh
./exe <liczba_klientów> <liczba_miejsc> [-info]
```
#### Wersja z Semaforami i Mutexami:
```sh
./sem_only/exe 10 5 -info
```

#### Wersja z Zmiennymi Warunkowymi i Kolejką
```sh
./conditional/exe 10 5 -info
```

## Autor
Projekt stworzony przez Mateusz Lengiewicz.
