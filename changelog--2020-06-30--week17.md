# Dziennik zmian - Tydzień 17 - 2020.06.30

Poniższa lista zawiera wybrane najistotniejsze zmiany z [gałęzi _master_
repozytorium](https://github.com/AdamJozwiak/PBL_Endless_Project/commits/master)
od dnia 2020.06.16, podzielone na kategorie.

## Zmiany

- **Gra**
  - Dodanie brakującej logiki niektórych stanów gry (np. zliczanie wyniku,
    pokazanie ekranu porażki, menu pauzy, itp.)
  - Dodanie efektu "odwrotnej atenuacji światła" przy przegranej
  - Dodatkowa modulacja kształtu modelu wroga
  - Poprawki funkcji modulacji natężenia światła w czasie
- **Optymalizacja**
  - Optymalizacja systemu ECS (przepisanie z map na tablice)
  - Redukcja liczby wierzchołków niektórych modeli (np. pułapki)
  - Optymalizacja grafu sceny
  - Dostosowanie parametrów shaderów (np. liczba sampli w parallax mappingu czy
    rozmyciu gaussowskim)
- **Cienie**
  - Dodanie mapowania cieni dla światła punktowego gracza

## Realizacja założeń

- **Iteracja III**
  - Generowanie cieni - **zrealizowane**
