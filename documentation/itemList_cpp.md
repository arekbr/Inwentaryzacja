# Dokumentacja dla pliku `itemList.cpp`

## Metadane
- **Plik**: itemList.cpp
- **Wersja**: 1.2.2
- **Data**: 2025-05-03
- **Autorzy**: Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK & ChatGPT & GROK

## Przegląd
Plik `itemList.cpp` zawiera implementację metod klasy `itemList`, która zarządza listą eksponatów w aplikacji inwentaryzacyjnej. Obsługuje połączenie z bazą danych MySQL, filtrowanie kaskadowe, wyświetlanie miniatur zdjęć, podgląd pełnoekranowy oraz operacje CRUD (dodawanie, edycja, usuwanie, klonowanie rekordów). Skórka graficzna i czcionki są zarządzane przez klasę `DatabaseConfigDialog`.

## Struktura kodu
Kod jest podzielony na następujące sekcje:

1. **Sekcja konstruktora**  
   - Inicjalizuje interfejs, model danych, model proxy, filtry i połączenia sygnałów-slotów.

2. **Sekcja destruktora**  
   - Zwalnia zasoby, zatrzymuje timery.

3. **Sekcja slotów**  
   - Obsługuje interakcje użytkownika: przyciski (dodaj, edytuj, usuń, klonuj, zakończ, o programie), zmiana zaznaczenia w tabeli, zapis rekordu, interakcje ze zdjęciami (podgląd, pełny ekran), zmiana filtrów.

4. **Sekcja metod prywatnych**  
   - Zarządza schematem bazy danych (`verifyDatabaseSchema`, `createDatabaseSchema`, `insertSampleData`).
   - Inicjalizuje i odświeża filtry (`initFilters`, `refreshFilters`, `updateFilterComboBoxes`).
   - Obsługuje zdarzenia dla okna podglądu zdjęć (`eventFilter`).

## Opis działania
1. **Inicjalizacja**:
   - Konstruktor ustawia interfejs użytkownika (`ui->setupUi`), łączy się z bazą MySQL, inicjalizuje model danych (`QSqlRelationalTableModel`) z relacjami do tabel `types`, `vendors`, `models`, `statuses`, `storage_places`.
   - Konfiguruje model proxy (`ItemFilterProxyModel`), tabelę (`QTableView`) i combo boxy dla filtrów.
   - Weryfikuje schemat bazy danych, tworząc go i wstawiając przykładowe dane, jeśli nie istnieje.
   - Ustawia połączenia sygnałów-slotów dla przycisków, tabeli i filtrów.

2. **Filtrowanie**:
   - Metoda `onFilterChanged` aktualizuje filtry w modelu proxy na podstawie wybranych wartości w combo boxach.
   - `updateFilterComboBoxes` odbudowuje listy combo boxów z uwzględnieniem kaskadowego filtrowania, pobierając unikalne wartości z bazy danych.
   - `initFilters` i `refreshFilters` wypełniają i odświeżają combo boxy danymi z bazy.

3. **Operacje CRUD**:
   - `onNewButtonClicked`, `onEditButtonClicked`, `onCloneButtonClicked`: Otwierają okno `MainWindow` w odpowiednim trybie (dodawanie, edycja, klonowanie).
   - `onDeleteButtonClicked`: Usuwa rekord po potwierdzeniu, wykonując zapytanie SQL i odświeżając model.
   - `onRecordSaved`: Odświeża listę po zapisaniu rekordu, zaznaczając zapisany rekord.

4. **Zdjęcia**:
   - `onTableViewSelectionChanged`: Pobiera zdjęcia z tabeli `photos` dla wybranego eksponatu i wyświetla miniatury w `QGraphicsView`.
   - `onPhotoHovered`: Pokazuje powiększone zdjęcie w oknie podglądu, pozycjonowanym względem miniatury.
   - `onPhotoUnhovered` i `eventFilter`: Zamykają okno podglądu, gdy kursor opuszcza miniaturę lub okno.
   - `onPhotoClicked`: Otwiera zdjęcie w trybie pełnoekranowym za pomocą `FullScreenPhotoViewer`.

5. **Baza danych**:
   - `verifyDatabaseSchema`: Sprawdza istnienie wymaganych tabel.
   - `createDatabaseSchema`: Tworzy tabele dla eksponatów, typów, producentów, modeli, statusów, miejsc przechowywania i zdjęć.
   - `insertSampleData`: Wstawia przykładowe dane z unikalnymi UUID.

6. **Interfejs**:
   - Tabela wyświetla dane z ukrytą kolumną UUID, nagłówkami dostosowanymi do lokalizacji i możliwością zaznaczania wierszy.
   - Przyciski obsługują operacje CRUD, zakończenie aplikacji i okno "O programie".
   - Timer `m_keepAliveTimer` wykonuje zapytania SELECT co 30 sekund, aby utrzymać połączenie z MySQL.
   - Timer `m_hoverCheckTimer` sprawdza pozycję kursora, zamykając okno podglądu, gdy kursor opuszcza odpowiedni obszar.

## Zależności
- **Biblioteki Qt**:
  - `QApplication`, `QSqlDatabase`, `QSqlQuery`, `QGraphicsScene`, `QMessageBox`, `QTimer`, `QComboBox`, `QSqlRelationalTableModel`, itp.: Obsługa interfejsu, bazy danych, zdarzeń i grafiki.
- **Nagłówki aplikacji**:
  - `itemList.h`: Definicja klasy.
  - `ItemFilterProxyModel.h`: Model proxy do filtrowania.
  - `fullscreenphotoviewer.h`: Pełnoekranowy podgląd zdjęć.
  - `mainwindow.h`: Okno edycji/dodawania rekordów.
  - `photoitem.h`: Klasa dla miniatur zdjęć.
- **Interfejs użytkownika**:
  - `ui_itemList.h`: Plik generowany przez Qt Designer.
- **Zasoby**:
  - Brak bezpośrednich zależności od zasobów Qt (np. QSS, czcionek).

## Uwagi
- Kod nie został zmodyfikowany, zgodnie z wymaganiami użytkownika.
- Implementacja zakłada użycie MySQL, ale aplikacja wspiera także SQLite (konfigurowane w `DatabaseConfigDialog`).
- Filtrowanie kaskadowe wymaga relacyjnej struktury bazy danych z poprawnymi kluczami obcymi.
- Zdjęcia są przechowywane jako `LONGBLOB` w tabeli `photos`, co może wpłynąć na wydajność przy dużej liczbie rekordów.
- Timer `m_keepAliveTimer` zapobiega timeoutom połączenia MySQL, ale zwiększa obciążenie bazy danych.
- Okno podglądu zdjęć jest bezramkowe i przezroczyste, co zapewnia estetyczny wygląd, ale wymaga ostrożnego zarządzania zdarzeniami.