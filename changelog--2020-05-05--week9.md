# Dziennik zmian - Tydzień 9 - 2020.05.05

Poniższa lista zawiera najistotniejsze zmiany z [gałęzi _master_
repozytorium](https://github.com/AdamJozwiak/PBL_Endless_Project/commits/master)
od dnia 2020.04.21, podzielone na kategorie.

## Zmiany

- **Billboarding**
  - Dodanie _geometry shaderów_
  - Dodanie _billboardingu_ wraz z przykładowym obiektem
- **Animacje**
  - Dodanie _animacji szkieletowej (skinning)_ wraz z testowym animowanym
    modelem _(mały wilk pośrodku sceny)_
- **Kolizje**
  - Dodanie _systemu kolizji_ _(AABB dla prostopadłościanów; sfery)_
  - Obliczanie _brył otaczających_ dla modeli
  - Emisja _zdarzeń_ w przypadku wystąpienia kolizji
  - Wyświetlanie _brył otaczających_
- **Parsowanie poziomów**
  - Parsowanie _dodatkowych plików z metadanymi (.prefab, .meta, .mat)_
  - Parsowanie _komponentów_ odpowiedzialnych za _rendering modeli_
  - _Integracja_ ładowanych komponentów Transform z _systemem renderingu_
  - Dodanie i wyświetlanie _prostej sceny testowej ładowanej z pliku_
- **Skrypty**
  - Dodanie prostego _skryptu do poruszania obiektem w scenie_
  - Dodanie _interakcji_ między dwoma obiektami w scenie - możliwe przesuwanie
    jednej małpki drugą po przytrzymaniu [Spacji]

## Realizacja założeń

- **Iteracja II**
  - Przemieszczanie postaci po scenie (skinning) - **zrealizowane**
  - Billboarding - **zrealizowane**
- **Iteracja III**
  - Generowanie cieni - _brak_
  - Testy - _brak_
  - Poprawa wydajności - _brak_
- **Iteracja IV**
  - Poprawki - _brak_

