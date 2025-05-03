# Dokumentacja klasy `itemList`

## 1. Cel klasy
Klasa `itemList` jest głównym widżetem aplikacji inwentaryzacyjnej dla retro komputerów, odpowiedzialnym za wyświetlanie i zarządzanie listą eksponatów. Jej główne funkcje to:
- **Wyświetlanie listy eksponatów**: Prezentuje dane w tabeli (`QTableView`) z kolumnami takimi jak nazwa, typ, producent, model, status i miejsce przechowywania.
- **Filtrowanie danych**: Umożliwia dynamiczne filtrowanie kaskadowe za pomocą combo boxów dla typu, producenta, modelu, statusu i miejsca przechowywania.
- **Zarządzanie rekordami**: Obsługuje operacje CRUD (Create, Read, Update, Delete) oraz klonowanie rekordów poprzez okna dialogowe (`MainWindow`).
- **Obsługa zdjęć**: Wyświetla miniatury zdjęć eksponatów, podgląd powiększonych zdjęć po najechaniu i pełnoekranowy podgląd po kliknięciu (`FullScreenPhotoViewer`).
- **Interakcja z bazą danych**: Integruje się z bazą danych MySQL, zarządzając schematem, danymi i połączeniem.

Klasa jest centralnym elementem interfejsu użytkownika, integrującym model danych (`QSqlRelationalTableModel`), model proxy (`ItemFilterProxyModel`) i funkcje interakcji z użytkownikiem.

## 2. Struktura kodu
Kod klasy `itemList` jest podzielony na logiczne sekcje w plikach `itemList.h` i `itemList.cpp`:

### 2.1 Plik `itemList.h`
- **Deklaracja klasy**: Definiuje klasę `itemList` dziedziczącą po `QWidget`, z metodami publicznymi, slotami prywatnymi i metodami prywatnymi.
- **Zmienne prywatne**:
  - `ui`: Wskaźnik na interfejs użytkownika (generowany z `ui_itemList.ui`).
  - `m_sourceModel`: Model źródłowy (`QSqlRelationalTableModel`) dla danych eksponatów.
  - `m_proxyModel`: Model proxy (`ItemFilterProxyModel`) do filtrowania danych.
  - Combo boxy (`filterTypeComboBox`, `filterVendorComboBox`, itp.) dla filtrów.
  - `m_currentRecordId`: ID aktualnie wybranego rekordu.
  - `m_previewWindow`: Okno podglądu powiększonego zdjęcia.
  - `m_previewHovered`, `m_currentHoveredItem`: Zmienne do zarządzania podglądem zdjęć.
  - `m_hoverCheckTimer`, `m_keepAliveTimer`: Timery do sprawdzania pozycji kursora i utrzymania połączenia z bazą.
- **Metody publiczne**: Konstruktor i destruktor.
- **Sloty prywatne**: Obsługa przycisków (dodaj, edytuj, usuń, klonuj, zakończ, o programie), zmian w tabeli, filtrów i interakcji ze zdjęciami.
- **Metody prywatne**: Weryfikacja i tworzenie schematu bazy, inicjalizacja i odświeżanie filtrów, obsługa zdarzeń.

### 2.2 Plik `itemList.cpp`
- **Inicjalizacja**:
  - Konfiguracja połączenia z bazą danych MySQL (`initializeDatabase`).
  - Inicjalizacja modelu danych i widoku (`initializeModelAndView`).
  - Podłączanie sygnałów i slotów (`initializeConnections`).
- **Zarządzanie bazą danych**:
  - Weryfikacja schematu (`verifyDatabaseSchema`).
  - Tworzenie schematu (`createDatabaseSchema`).
  - Wstawianie przykładowych danych (`insertSampleData`).
- **Filtrowanie**:
  - Inicjalizacja filtrów (`initFilters`).
  - Odświeżanie filtrów (`refreshFilters`).
  - Kaskadowe aktualizowanie combo boxów (`updateFilterComboBoxes`).
- **Interakcja ze zdjęciami**:
  - Wyświetlanie miniatur (`onTableViewSelectionChanged`).
  - Podgląd powiększonego zdjęcia (`onPhotoHovered`).
  - Pełnoekranowy podgląd (`onPhotoClicked`).
  - Ukrywanie podglądu (`onPhotoUnhovered`, `eventFilter`).
- **Operacje CRUD**:
  - Dodawanie (`onNewButtonClicked`).
  - Edycja (`onEditButtonClicked`).
  - Klonowanie (`onCloneButtonClicked`).
  - Usuwanie (`onDeleteButtonClicked`).
- **Inne**:
  - Zamykanie aplikacji (`onEndButtonClicked`).
  - Wyświetlanie informacji o programie (`onAboutClicked`).
  - Odświeżanie listy (`refreshList`, `onRecordSaved`).

Każda metoda i sekcja jest udokumentowana za pomocą komentarzy Doxygen, zawierających szczegółowe informacje o parametrach, zwracanych wartościach, zależnościach i uwagach.

## 3. Zależności
Klasa `itemList` korzysta z następujących bibliotek i zasobów:
- **Biblioteki Qt**:
  - `QWidget`, `QComboBox`, `QTableView`, `QGraphicsView`, `QPushButton`, `QMessageBox`, itp.: Do budowy interfejsu użytkownika.
  - `QSqlDatabase`, `QSqlQuery`, `QSqlRelationalTableModel`: Do zarządzania bazą danych MySQL.
  - `QTimer`: Do obsługi podglądu zdjęć i utrzymania połączenia z bazą.
  - `QPixmap`, `QGraphicsScene`, `QGraphicsPixmapItem`: Do wyświetlania zdjęć.
- **Własne komponenty**:
  - `ItemFilterProxyModel`: Model proxy do filtrowania danych.
  - `FullScreenPhotoViewer`: Okno pełnoekranowego podglądu zdjęć.
  - `MainWindow`: Okno dialogowe do dodawania, edycji i klonowania rekordów.
  - `PhotoItem`: Klasa do obsługi miniatur zdjęć.
  - `ui_itemList.h`: Plik UI wygenerowany przez Qt Designer.
- **Zewnętrzne zasoby**:
  - Baza danych MySQL z tabelami: `eksponaty`, `types`, `vendors`, `models`, `statuses`, `storage_places`, `photos`.
  - Plik UI (`ui_itemList.ui`) definiujący układ interfejsu.
- **Zewnętrzne narzędzia**:
  - Qt Designer: Do projektowania interfejsu użytkownika.
  - Doxygen: Do generowania dokumentacji.

## 4. Instrukcje użycia
1. **Kompilacja**:
   - Upewnij się, że projekt Qt jest poprawnie skonfigurowany z modułami Core, Gui, Widgets i Sql (Qt 5 lub Qt 6).
   - Pliki `itemList.h`, `itemList.cpp`, `ItemFilterProxyModel.h`, `ItemFilterProxyModel.cpp`, `fullscreenphotoviewer.h`, `fullscreenphotoviewer.cpp`, `mainwindow.h`, `mainwindow.cpp`, `photoitem.h`, `photoitem.cpp` oraz plik UI (`ui_itemList.ui`) muszą być dołączone do projektu.
   - Skonfiguruj połączenie z bazą danych MySQL w kodzie (`db.setHostName`, `db.setUserName`, itp.) lub przez plik konfiguracyjny.
2. **Uruchomienie**:
   - Stwórz instancję klasy `itemList` jako główny widżet aplikacji:
     ```cpp
     #include "itemList.h"
     #include <QApplication>
     int main(int argc, char *argv[]) {
         QApplication app(argc, argv);
         itemList list;
         list.show();
         return app.exec();
     }
     ```
   - Aplikacja automatycznie połączy się z bazą danych, zweryfikuje schemat, a w razie potrzeby stworzy tabele i wstawi przykładowe dane.
3. **Interakcja z użytkownikiem**:
   - **Filtrowanie**: Używaj combo boxów (typ, producent, model, status, miejsce przechowywania) do ograniczania widocznych rekordów.
   - **Miniatury zdjęć**: Wybierz rekord w tabeli, aby wyświetlić miniatury zdjęć. Najedź na miniaturę, aby zobaczyć podgląd, lub kliknij, aby otworzyć pełnoekranowy podgląd.
   - **Operacje CRUD**:
     - **Dodaj**: Kliknij przycisk "Nowy", aby otworzyć okno dodawania rekordu.
     - **Edytuj**: Zaznacz rekord i kliknij "Edytuj", aby otworzyć okno edycji.
     - **Klonuj**: Zaznacz rekord i kliknij "Klonuj", aby otworzyć okno klonowania.
     - **Usuń**: Zaznacz rekord i kliknij "Usuń", potwierdzając operację.
   - **Zakończ**: Kliknij "Zakończ", aby zamknąć aplikację.
   - **O programie**: Kliknij "O programie", aby wyświetlić informacje o aplikacji.
4. **Generowanie dokumentacji**:
   - Użyj narzędzia Doxygen z plikiem konfiguracyjnym `Doxyfile`, ustawiając:
     ```
     INPUT = ./
     RECURSIVE = YES
     FILE_PATTERNS = *.cpp *.h
     ```
   - Uruchom `doxygen Doxyfile`, aby wygenerować dokumentację HTML lub LaTeX.

## 5. Uwagi
- **Baza danych**: Aplikacja wymaga działającego serwera MySQL z poprawnie skonfigurowanym dostępem. Nieprawidłowe dane logowania lub brak serwera spowodują błąd i zamknięcie aplikacji.
- **Schemat bazy**: Schemat jest automatycznie tworzony, jeśli nie istnieje, ale wymaga uprawnień do tworzenia tabel w bazie danych.
- **Zdjęcia**: Zdjęcia są przechowywane jako `LONGBLOB` w tabeli `photos`. Nieprawidłowe dane obrazów mogą powodować błędy ładowania.
- **Filtrowanie kaskadowe**: Combo boxy są dynamicznie aktualizowane w zależności od wybranych filtrów, co może wpłynąć na wydajność przy dużych zbiorach danych.
- **Podgląd zdjęć**: Mechanizm podglądu używa timera do sprawdzania pozycji kursora, co może wymagać optymalizacji dla bardzo dużych zdjęć lub słabszych maszyn.
- **Skalowalność**: Klasa jest zaprojektowana dla średniej wielkości zbiorów danych. Przy bardzo dużych bazach danych może być potrzebna optymalizacja zapytań SQL.
- **Zewnętrzne zależności**: Klasy `MainWindow`, `PhotoItem` i `DatabaseConfigDialog` muszą być zaimplementowane i dostępne w projekcie.

## 6. Zobacz także
- `itemList.h`: Deklaracja klasy.
- `itemList.cpp`: Implementacja klasy.
- `ItemFilterProxyModel.h`, `ItemFilterProxyModel.cpp`: Model proxy do filtrowania.
- `fullscreenphotoviewer.h`, `fullscreenphotoviewer.cpp`: Pełnoekranowy podgląd zdjęć.
- `mainwindow.h`, `mainwindow.cpp`: Okno dialogowe do edycji rekordów.
- `photoitem.h`, `photoitem.cpp`: Klasa do obsługi miniatur zdjęć.
- [Qt Documentation](https://doc.qt.io): Dokumentacja frameworku Qt.
- [Doxygen](https://www.doxygen.nl): Narzędzie do generowania dokumentacji.