# Dziennik zmian - Tydzień 13 - 2020.06.02

Poniższa lista zawiera najistotniejsze zmiany z [gałęzi _master_
repozytorium](https://github.com/AdamJozwiak/PBL_Endless_Project/commits/master)
od dnia 2020.05.19, podzielone na kategorie.

## Zmiany

- **Skrypty**
  - Dodanie portu struktury oraz treści skryptów z prototypu w Unity
  - Dodanie ładowania informacji o kamerze z plików sceny
- **Korekcja kolorów**
  - Dodanie korekcji gamma, jasności/kontrastu oraz krzywych
- **ECS**
  - Dodanie obsługi statusu obiektu (aktywny/nieaktywny)
  - Dodanie możliwość znajdowania encji po nazwie/tagu
- **Scena**
  - Dodanie możliwość obsługi wielu świateł punktowych
- **Optymalizacja**
  - Zoptymalizowanie ładowanie zasobów poprzez zapewnienie, że dany zasób
    ładuje się do pamięci tylko raz
  - Dodanie frustum cullingu
  - Dodanie grafu sceny z "brudną" flagą
- **Dźwięk**
  - Dodanie system dźwięków
- **UI**
  - Dodanie renderowania tekstu
  - Dodanie podstawowego przycisku
- **Blending**
  - Zmiana wcześniejszej binarnej obsługi przezroczystości (discard) na pełne
    testy przezroczystości
  - Wykorzystanie testu przezroczystości przy ulepszeniu wyglądu efektu płomienia
- **Rendering**
  - Dodanie parallax mappingu
  - Wsparcie dla skalowania niejednorodnego

## Realizacja założeń

- **Iteracja III**
  - Testy - **zrealizowane**: _testy przezroczystości_
  - Poprawa wydajności - **zrealizowane**: _frustum culling, graf sceny z
    "brudną" flagą_
  - Generowanie cieni - **w trakcie realizacji**
