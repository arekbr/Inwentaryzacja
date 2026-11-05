# Lista eksponatów: przesuwana granica między tabelą a panelem opisu

Maszyna: Linux (Debian 13). Plik: `ZADANIA-lista-przesuwana-granica.md` w katalogu głównym repo.

**CO:** użytkownik chwyta granicę między tabelą eksponatów a panelem zdjęcia i opisu, przesuwa ją myszą i widzi więcej wierszy tabeli albo więcej opisu. Po ponownym uruchomieniu aplikacji granica jest tam, gdzie ją zostawił.

**GOTOWE GDY:**
- Przy oknie 1203×896 (wysokość, która przed zmianą dawała 0 pełnych wierszy) tabela pokazuje od razu co najmniej 6 pełnych wierszy. Nowe okno startowe bez zapisanej geometrii ma 1203×820 i co najmniej 5 pełnych wierszy.
- Po przeciągnięciu granicy w dół tabela ma co najmniej 75% wysokości obszaru tabela + panel, a przycisków nie ubywa i nie zachodzą na siebie.
- Po przeciągnięciu w górę panel opisu ma co najmniej 60% tego obszaru.
- Po zamknięciu i ponownym otwarciu okna wysokość tabeli różni się od zapisanej najwyżej o 2 px.
- Po przesunięciu granicy i po zmianie rozmiaru okna zdjęcia mieszczą się w panelu w całości, bez pasków przewijania i bez ponownego czytania z bazy. Po powiększeniu panelu miniatury są przeliczone do nowego rozmiaru, a nie rozciągnięte.

**SPRAWDZĘ TAK:** `InwentaryzacjaTests` z testami `itemList_splitter*` (ruch prawdziwymi zdarzeniami myszy na uchwycie). Kontrola negatywna: te same testy CZERWONE na kodzie sprzed zmiany. Potem zrzuty i przeciągnięcie `xdotool` na niewidocznym ekranie, na prawdziwej bazie, 1203×896 i 1920×1048.

**BRZEGI:**
- Pierwsze uruchomienie, bez zapisanego położenia: proporcja domyślna 3:2 na korzyść tabeli.
- Uszkodzone lub puste dane położenia w ustawieniach: `restoreState` zwraca `false`, też proporcja domyślna, bez błędu.
- Granica ciągnięta do końca: żadna część nie znika całkowicie, oba panele trzymają minimum.
- Stare ustawienia z zapisaną geometrią okna, ale bez położenia granicy: geometria działa jak dotąd, granica domyślna.
- Brak zaznaczonego eksponatu lub eksponat bez zdjęcia podczas przesuwania: nic się nie dzieje, bez awarii.
- Zmiana wysokości okna: proporcja tabela do panelu zostaje (±2 punkty procentowe). Czynnik rozciągania tylko na tabeli odrzucony: `QSplitter` traktuje wtedy `setSizes` panelu jako rozmiar bezwzględny, więc domyślna proporcja przestałaby działać.
- Zamknięcie przyciskiem `Koniec` (idzie przez `qApp->quit()`): położenie ma się zapisać tak samo jak przy zamknięciu okna, sprawdzone kliknięciem w T04.

**POZA ZAKRESEM:** zwijane filtry, zmiana kolejności lub wyglądu przycisków, minimalna wysokość okna na niskich ekranach (była 896 px, jest 820 px), macOS i Windows poza zielonym CI (CI nie uruchamia pełnego zestawu testów, tylko buduje i sprawdza sterownik bazy).

**ŚWIADOMIE ODRZUCONE z przeglądu kodu:** miniatury o zerowej wysokości, bo wymagają co najmniej 16 zdjęć w jednej kolumnie (w bazie najwyżej 11) i nie powodują awarii.

**ZAŁOŻENIA:**
- Decyzja usera 5.11.2026: przesuwana granica, spośród trzech wariantów. Zamknięte.
- Kolumna przycisków przechodzi na prawo od CAŁEGO obszaru tabela + panel. Pomiar 5.11: dolny pas ma ~372 px przy oknie 896 i 1048 px, czyli jego minimum wyznacza kolumna 9 przycisków, a nie panel opisu. Bez tej zmiany granicy nie dałoby się przesunąć w dół. Koszt: tabela węższa o szerokość kolumny, ok. 120 px.
- Nowe wydanie 1.7.2, instalacja na Linuksie; macOS zostaje na 1.7.1 do osobnego zlecenia.

**JAK:** `QSplitter` w `forms/itemList.ui`, położenie w `itemList/splitter` w ini obok `itemList/geometry`. Zmiana rozmiaru panelu zdjęć (filtr zdarzeń na `viewport`) uruchamia po 100 ms przeliczenie miniatur z oryginałów trzymanych w scenie. Pierwsza wersja z samym `fitInView` rozciągała miniatury i nie reagowała na zmianę rozmiaru okna; odrzucona po przeglądzie kodu.

Uzasadnienie warstwy: `QSplitter` | przesuwana granica z wyboru usera | prostsze: stała proporcja, odrzucona przez usera; własny uchwyt, odrzucony, bo to samo robi gotowa klasa.

## Przyrosty

- [x] T01 Testy czerwone w `tests/repository_tests.cpp`
      SPRAWDZENIE: testy `itemList_splitter*` padają na kodzie sprzed zmiany, reszta zestawu zielona.
      WYNIK: 5 czerwonych, 42 zielone, stary układ 0 pełnych wierszy. Pierwsza wersja testu wierszy była zielona na starym kodzie, bo nie ładowała skórki Standard; poprawione.
- [x] T02 Granica i kolumna przycisków w `forms/itemList.ui`, stan w `src/itemList.cpp`
      SPRAWDZENIE: testy wymiarów i zapamiętywania zielone; zrzut pokazuje co najmniej 5 wierszy na nowym oknie startowym.
      WYNIK: 1203×820 na prawdziwej bazie: 5 pełnych wierszy i część szóstego.
- [x] T03 Zdjęcie dopasowane po przesunięciu w `src/itemList.cpp`
      SPRAWDZENIE: test zdjęcia zielony; na prawdziwej bazie eksponat ze zdjęciem po przeciągnięciu mieści się w panelu.
      WYNIK: przed dopasowaniem scena 416×369 przy panelu 420×84; po poprawce zielono. Komplet: 47 zaliczonych, 0 błędów, 2 pominięte celowo. Na czystym `dev` te same testy: 5 czerwonych, 42 zielone. Mutacje (bez przeliczania, bez zapisu, bez odczytu, bez proporcji domyślnej, bez filtra ramki) dają czerwień w odpowiednim teście.
- [x] T04 Próba na prawdziwej bazie (`xdotool`, kopia ini) i uchwyt widoczny w 6 skórkach
      SPRAWDZENIE: zrzuty przed i po przeciągnięciu, wiersze policzone; uchwyt widać na każdej skórce.
      WYNIK: 1203×820: 5 wierszy na starcie, 7 maksymalnie; 1920×1048: 15 maksymalnie. `Koniec` zapisuje stan, po restarcie okno i granica bez zmian (różnica 0 px). Kropki uchwytu w 6 skórkach, zdjęcie w całości w 3 położeniach.
- [x] T05 Wydanie 1.7.2: `version.txt`, PR do `dev`, PR do `main`, tag
      SPRAWDZENIE: CI zielone na 4 platformach, paczki w wydaniu, skrót zgodny z GitHubem.
      WYNIK: PR #47, #49, #48, tag `v1.7.2`, 7 plików, skrót paczki Debian zgodny. Po drodze: GitHub wycofał obrazy macOS 14, więc paczka ARM buduje się na `macos-15`, a jej minimalny system wzrósł z 14.0 na 15.0. Budowanie tagu wymagało powtórki zadania Intel (`stapler` stracił połączenie z serwerem Apple).
- [x] T06 Instalacja na Linuksie
      SPRAWDZENIE: `dpkg -s` 1.7.2, aplikacja z ustawieniami usera otwiera listę, granica działa.
      WYNIK: `dpkg -V` czysto; kopia ustawień z oknem 1203×844: 6 wierszy na starcie, 8 maksymalnie, po restarcie różnica 0 px; lista 1871 / 1871. Paczki macOS i Windows zbudowane w CI, nieuruchamiane.
