# Tworzenie nowego chunka

## Wstęp

- Utwórz w głównej scenie nowy _GameObject_ i nazwij go, tak jak chcesz nazwać
  nowy _chunk_.

- Dodaj do niego jako dziecko prefab _Chunk Base_

- Przeciągnij powstały chunk do _Chunks Editable_ - w ten sposób stworzysz nowy
  prefab.

- Usuń instancję prefabu chunka z głównej sceny

- Otwórz powstały prefab chunka z _Chunks Editable_

## Edycja

- W _Ground_ umieść drogę powstałą z prefabów _Part_

- W _Walls_ umieść ściany

- W _Enemy Spawn Points_ umieść spawn pointy

- W _Obstacles_ umieść przeszkody, które gracz będzie musiał omijać

- W _Decorations_ umieść kamienie przez które gracz docelowo będzie mógł
  przebiec ale będą ulepszały scenę wizualnie

- W _Troches_ umieść pochodnie

- W _Traps_ umieść pułapki

- W _Waterfalls_ umieść wodospady

- W _Enemy Boundaries_ umieść granice dla AI

- Jeśli jakiegoś elementu nie trzeba umieszczać w danym chunku, bo np. w nim
  nie będzie przeciwników, to go nie umieszczaj

- Jeśli skończysz pracę nad prefabem, zapisz go i przejdź do głównej sceny

## Odpakowywanie

- Utwórz instancję prefabu chunka w głównej scenie, kliknij na nim _Unpack
  Prefab Completely_ a następnie przeciągnij tę instancję do _Chunks Completely
  Unpacked_ (w _Chunks Editable_ trzymamy chunki w wersji do edycji i tylko te
  edytujemy; w _Chunks Completely Unpacked_ trzymamy chunki do załadowania przez
  silnik; jeśli wielokrotnie update'ujemy chunka przy pracy z silnikiem, należy
  usunąć dany prefab z folderu _Unpacked_ i wykonać ponownie czynności z tego
  punktu z tym odpakowywaniem wersji edytowalnej; **UWAGA! - uważaj, żeby
  przypadkiem nie usunąć chunka z Chunks Editable, łatwo się pomylić - polecam
  robienie częstych kopii, lub checkpointowych commitów**)

## Ustawianie właściwości

- Po utworzeniu prefabu, w scenie głownej w _Chunk Start-End Properties_ utwórz
  _GameObject_ o nazwie twojego chunka i nadaj mu tag _ChunkStartEndProperty_ -
  następnie w jego transformie zakoduj: w rotacji ustaw 1 tam gdzie zaczyna się
  droga, natomiast w skali tam gdzie się kończy; X - lewo, Y - środek, Z -
  prawo. Np. jeśli chunk zaczyna się drogą z lewej strony z kończy z prawej, to
  rotacja będzie 1,0,0 a skala 0,0,1

- Podobnie z _Chunk Length Properties_, utwórz _GameObject_ o nazwie prefaba,
  nadaj mu tak _ChunkLengthProperty_, tylko tym razem zakoduj jego długość (ile
  razy użyłeś instancji Part w drodze) w komponencie X skali.

## Testowanie

- W celu przetestowania prefabów w silniku, należy najpierw uruchomić
  prepare-assets.bat

## Pytania

- W razie wątpliwości - patrz na _Chunk 1_ bądź pytaj na priv
