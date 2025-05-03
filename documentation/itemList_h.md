# Dokumentacja dla pliku `itemList.h`

## Metadane
- **Plik**: itemList.h
- **Wersja**: 1.2.2
- **Data**: 2025-05-03
- **Autorzy**: Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK & ChatGPT & GROK

## Przegląd
Plik `itemList.h` definiuje klasę `itemList`, która jest głównym widgetem aplikacji inwentaryzacyjnej, odpowiedzialnym za wyświetlanie i zarządzanie listą eksponatów. Klasa obsługuje filtrowanie kaskadowe, operacje CRUD (Create, Read, Update, Delete), wyświetlanie miniatur zdjęć, podgląd pełnoekranowy oraz interakcje użytkownika. Skórka graficzna i czcionki są zarządzane przez klasę `DatabaseConfigDialog`.

## Struktura kodu
Plik nagłówkowy zawiera następujące elementy:

1. **Deklaracja klasy `itemList`**  
   - Dziedziczy po `QWidget`.
   - Używa makra `Q_OBJECT` dla wsparcia mechanizmu sygnałów i slotów Qt.

2. **Metody publiczne**  
   - Konstruktor: Inicjalizuje interfejs, model danych i filtry.
   - Destruktor: Zwalnia zasoby.

3. **Sloty prywatne**  
   - `onNewButtonClicked`: Otwiera okno dodawania nowego eksponatu.
   - `onEditButtonClicked`: Otwiera okno edycji wybranego eksponatu.
   - `onEndButtonClicked`: Zamyka aplikację.
   - `onDeleteButtonClicked`: Usuwa wybrany eksponat.
   - `onAboutClicked`: Wyświetla okno "O programie".
   - `onTableViewSelectionChanged`: Aktualizuje miniatury zdjęć dla wybranego eksponatu.
   - `onRecordSaved`: Odświeża listę po zapisaniu rekordu.
   - `refreshList`: Odświeża listę eksponatów.
   - `onPhotoHovered`: Wyświetla podgląd zdjęcia.
   - `onPhotoUnhovered`: Ukrywa podgląd zdjęcia.
   - `onPhotoClicked`: Otwiera zdjęcie w trybie pełnoekranowym.
   - `onCloneButtonClicked`: Otwiera okno klonowania eksponatu.
   - `onFilterChanged`: Aktualizuje filtry combo boxów.

4. **Metody prywatne**  
   - `verifyDatabaseSchema`: Weryfikuje schemat bazy danych.
   - `createDatabaseSchema`: Tworzy schemat bazy danych.
   - `insertSampleData`: Wstawia przykładowe dane.
   - `initFilters`: Inicjalizuje filtry combo boxów.
   - `refreshFilters`: Odświeża filtry combo boxów.
   - `updateFilterComboBoxes`: Odbudowuje listy combo boxów dla filtrowania kaskadowego.
   - `eventFilter`: Obsługuje zdarzenia dla okna podglądu zdjęć.

5. **Zmienne prywatne**  
   - `ui`: Wskaźnik na interfejs użytkownika.
   - `m_sourceModel`: Model źródłowy (`QSqlRelationalTableModel`).
   - `m_proxyModel`: Model proxy (`ItemFilterProxyModel`).
   - `filterTypeComboBox`, `filterVendorComboBox`, `filterModelComboBox`, `filterStatusComboBox`, `filterStorageComboBox`: Combo boxy dla filtrów.
   - `m_currentRecordId`: ID wybranego rekordu.
   - `m_previewWindow`: Okno podglądu zdjęcia.
   - `m_previewHovered`: Flaga stanu podglądu.
   - `m_currentHoveredItem`: Aktualnie podświetlony element zdjęcia.
   - `m_hoverCheckTimer`: Timer sprawdzania pozycji kursora.
   - `m_keepAliveTimer`: Timer utrzymujący połączenie z bazą danych.

## Opis działania
- **Inicjalizacja**: Konstruktor ustawia interfejs użytkownika, model danych, model proxy, filtry i połączenia sygnałów-slotów. Weryfikuje schemat bazy danych, a w razie potrzeby tworzy go i wstawia przykładowe dane.
- **Filtrowanie**: Używa `ItemFilterProxyModel` do dynamicznego filtrowania danych za pomocą combo boxów. Filtrowanie kaskadowe aktualizuje dostępne opcje w combo boxach.
- **Operacje CRUD**: Obsługuje dodawanie, edytowanie, usuwanie i klonowanie rekordów poprzez okna `MainWindow`.
- **Zdjęcia**: Wyświetla miniatury zdjęć w `QGraphicsView`, umożliwia podgląd po najechaniu i pełnoekranowy widok po kliknięciu (`FullScreenPhotoViewer`).
- **Interfejs**: Tabela (`QTableView`) wyświetla przefiltrowane dane, z ukrytą kolumną UUID i nagłówkami dostosowanymi do lokalizacji.
- **Baza danych**: Współpracuje z MySQL (lub SQLite, skonfigurowanym w `DatabaseConfigDialog`), zarządzając schematem i danymi.

## Zależności
- **Biblioteki Qt**:
  - `QWidget`, `QComboBox`, `QSqlRelationalTableModel`, `QSettings`, `QLabel`, `QItemSelection`: Obsługa interfejsu, danych i interakcji.
- **Nagłówki aplikacji**:
  - `ItemFilterProxyModel.h`: Model proxy do filtrowania.
  - `photoitem.h`: Klasa dla miniatur zdjęć.
- **Interfejs użytkownika**:
  - `ui_itemList.h`: Plik generowany przez Qt Designer.
- **Zasoby**:
  - Brak bezpośrednich zależności od zasobów Qt (np. QSS, czcionek).

## Uwagi
- Kod nie został zmodyfikowany, zgodnie z wymaganiami użytkownika.
- Klasa integruje wiele komponentów aplikacji, będąc jej centralnym elementem.
- Filtrowanie kaskadowe wymaga poprawnego schematu bazy danych z relacjami.
- Timer `m_keepAliveTimer` zapobiega rozłączeniu z bazą MySQL w przypadku długiej bezczynności.
- Zdjęcia są przechowywane jako BLOB w tabeli `photos`.