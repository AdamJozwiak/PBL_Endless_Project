# Dziennik zmian - Tydzień 7 - 2020.04.21

Poniższa lista zawiera najistotniejsze zmiany z [gałęzi _develop_ repozytorium](https://github.com/AdamJozwiak/PBL_Endless_Project/commits/develop), podzielone na kategorie.

## Zmiany

- **Konfiguracja**
  - Dodanie i konfiguracja _bibliotek: assimp (do zasobów), imgui (do debuggowania), yaml-cpp (do parsowania plików scen)_
- **Rendering**
  - Tworzenie _okna_
  - Obsługa _wyjątków_ oraz _wejścia z klawiatury i myszki_
  - Dodanie _podstawowych operacji graficznych korzystających z DirectX_
  - Dodanie _narzędzi debuggowania DirectX_
  - Narysowanie _podstawowego trójkąta_
  - Implementacja _hierarchii renderowalnych obiektów_
  - Dodanie klas odpowiedzialnych za _tekstury_
  - Dodanie zbioru klas rysujących _podstawowe kształty_
  - Dodanie _kamery_
- **ECS**
  - Implementacja _wzorca ECS_
  - Dodanie _podstawowych komponentów_
  - Dodanie _komponentów odnoszących się do kolizji_
  - Integracja _renderingu z odpowiednim systemem_
  - Dodanie _systemu scen_
  - Implementacja _systemu skryptów (.dll)_
- **Poziomy**
  - Implementacja _podstawowego parsowania poziomów z plików Unity_
- **Oświetlenie**
  - Dodanie _oświetlenia Phonga_
  - Dodanie _światła punktowego_
- **Modele i animacje**
  - Dodanie _klas do składowania danych modelu_
  - Ładowanie _podstawowych plików modeli (.obj, .dae, .gltf)_
  - Ładowanie _modeli zawierających pod-siatki_
  - _Poruszanie poszczególnymi pod-siatkami modelu za pomocą GUI_
  - Ładowanie _kości z modelu_

## Realizacja założeń

- **Iteracja II**
  - Importowanie siatki postaci - **zrealizowane** _dla statycznych modeli_
  - Przemieszczanie postaci po scenie (skinning) - **w trakcie realizacji**: _trwają prace nad ładowaniem klatek kluczowych animacji oraz tworzeniem shadera odpowiedzialnego za skinning_
  - Teksturowanie - **zrealizowane** _dla podstawowego teksturowania obiektów, w planach rozszerzenie funkcjonalności do PBR_
  - Billboarding - **w trakcie realizacji**: _obecne jest już wyświetlanie płaszczyzn w przestrzeni, pozostaje dodanie odpowiedniej ich orientacji_
- **Iteracja III**
  - Oświetlenie - **zrealizowane** _dla podstawowego modelu Phonga, w planach realizacja PBR_
  - Generowanie cieni - _brak_
  - Testy - _brak_
  - Poprawa wydajności - _brak_
- **Iteracja IV**
  - Poprawki - _brak_
