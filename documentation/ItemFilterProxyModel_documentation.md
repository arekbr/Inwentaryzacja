# Dokumentacja klasy `ItemFilterProxyModel`

## 1. Cel klasy
Klasa `ItemFilterProxyModel` jest modelem proxy w aplikacji inwentaryzacyjnej, który umożliwia dynamiczne filtrowanie listy eksponatów na podstawie wybranych kryteriów, takich jak:
- Typ eksponatu (np. komputer, konsola).
- Producent (np. Commodore, Atari).
- Model (np. Amiga 500, ZX Spectrum).
- Status (np. sprawny, uszkodzony).
- Miejsce przechowywania (np. magazyn, wystawa).

Klasa dziedziczy po `QSortFilterProxyModel` i współpracuje z modelem źródłowym (np. `QSqlRelationalTableModel`), aby ograniczyć wyświetlane wiersze w widoku (np. `QTableView`) na podstawie filtrów ustawionych przez użytkownika za pomocą combo boxów.

## 2. Struktura kodu
Kod klasy `ItemFilterProxyModel` jest podzielony na logiczne sekcje w plikach `ItemFilterProxyModel.h` i `ItemFilterProxyModel.cpp`:

### 2.1 Plik `ItemFilterProxyModel.h`
- **Deklaracja klasy**: Definiuje klasę `ItemFilterProxyModel` z metodami do ustawiania filtrów (`setTypeFilter`, `setVendorFilter`, itp.) i metodą `filterAcceptsRow` do implementacji logiki filtrowania.
- **Zmienne prywatne**: Przechowują wartości filtrów (`m_type`, `m_vendor`, `m_model`, `m_status`, `m_storage`).
- **Metody publiczne**: Umożliwiają ustawianie filtrów dla każdej kategorii.
- **Metoda chroniona**: `filterAcceptsRow` określa, które wiersze modelu źródłowego są widoczne.

### 2.2 Plik `ItemFilterProxyModel.cpp`
- **Inicjalizacja modelu** (`initializeModel`): Konfiguruje model proxy, ustawiając niewrażliwość na wielkość liter.
- **Logika filtrowania** (`applyFilters`): Implementuje sprawdzanie zgodności wierszy modelu źródłowego z filtrami.
- **Metody ustawiania filtrów**: Zapisują wartości filtrów i odświeżają widok poprzez `invalidateFilter()`.
- **Metoda `filterAcceptsRow`**: Wywołuje `applyFilters` do oceny wierszy.

Każda metoda i sekcja jest udokumentowana za pomocą komentarzy Doxygen, zawierających szczegółowe informacje o parametrach, zwracanych wartościach, zależnościach i uwagach.

## 3. Zależności
Klasa `ItemFilterProxyModel` korzysta z następujących bibliotek i zasobów:
- **Biblioteki Qt**:
  - `QSortFilterProxyModel`: Bazowa klasa dla modelu proxy.
  - `QModelIndex`: Obsługa indeksów modelu źródłowego.
- **Własne komponenty**:
  - Model źródłowy, np. `QSqlRelationalTableModel`, dostarczający dane w kolumnach: 2 (typ), 3 (producent), 4 (model), 9 (status), 10 (miejsce przechowywania).
  - Widok, np. `QTableView`, do wyświetlania przefiltrowanych danych.
  - Combo boxy w interfejsie użytkownika (np. w klasie `itemList`) do ustawiania filtrów.
- **Zewnętrzne narzędzia**:
  - Qt Designer: Opcjonalnie, do projektowania interfejsu z combo boxami.
  - Doxygen: Do generowania dokumentacji.

## 4. Instrukcje użycia
1. **Kompilacja**:
   - Upewnij się, że projekt Qt jest poprawnie skonfigurowany z modułami Core, Gui, Widgets i Sql (Qt 5 lub Qt 6).
   - Pliki `ItemFilterProxyModel.h` i `ItemFilterProxyModel.cpp` muszą być dołączone do projektu.
2. **Uruchomienie**:
   - Stwórz instancję klasy `ItemFilterProxyModel` i ustaw model źródłowy (np. `QSqlRelationalTableModel`):
     ```cpp
     QSqlRelationalTableModel *sourceModel = new QSqlRelationalTableModel(this);
     ItemFilterProxyModel *proxyModel = new ItemFilterProxyModel(this);
     proxyModel->setSourceModel(sourceModel);
     ```
   - Podłącz model proxy do widoku (np. `QTableView`):
     ```cpp
     ui->tableView->setModel(proxyModel);
     ```
   - Ustaw filtry za pomocą metod publicznych, np.:
     ```cpp
     proxyModel->setTypeFilter("Komputer");
     proxyModel->setVendorFilter("Commodore");
     ```
3. **Integracja z interfejsem**:
   - Podłącz combo boxy w interfejsie użytkownika do metod ustawiania filtrów:
     ```cpp
     connect(ui->typeComboBox, &QComboBox::currentTextChanged, proxyModel, &ItemFilterProxyModel::setTypeFilter);
     connect(ui->vendorComboBox, &QComboBox::currentTextChanged, proxyModel, &ItemFilterProxyModel::setVendorFilter);
     ```
   - Upewnij się, że combo boxy zawierają odpowiednie wartości (np. listę typów, producentów) pobraną z bazy danych.
4. **Generowanie dokumentacji**:
   - Użyj narzędzia Doxygen z plikiem konfiguracyjnym `Doxyfile`, ustawiając:
     ```
     INPUT = ./
     RECURSIVE = YES
     FILE_PATTERNS = *.cpp *.h
     ```
   - Uruchom `doxygen Doxyfile`, aby wygenerować dokumentację HTML lub LaTeX.

## 5. Uwagi
- **Model źródłowy**: Klasa zakłada, że model źródłowy dostarcza dane w kolumnach: 2 (typ), 3 (producent), 4 (model), 9 (status), 10 (miejsce przechowywania). Nieprawidłowe mapowanie kolumn może spowodować błędne filtrowanie.
- **Niewrażliwość na wielkość liter**: Filtrowanie jest niewrażliwe na wielkość liter, co ułatwia użytkowanie, ale może wymagać dostosowania, jeśli wielkość liter jest istotna.
- **Puste filtry**: Pusty ciąg w filtrze oznacza brak ograniczeń dla danej kategorii, co pozwala na elastyczne filtrowanie.
- **Skalowalność**: Klasa może być rozszerzona o dodatkowe filtry poprzez dodanie nowych zmiennych i metod, np. dla daty zakupu lub kategorii eksponatu.
- **Wydajność**: Filtrowanie jest wykonywane dla każdego wiersza modelu źródłowego przy każdej zmianie filtru, co może wpłynąć na wydajność przy bardzo dużych zbiorach danych.

## 6. Zobacz także
- `ItemFilterProxyModel.h`: Deklaracja klasy.
- `ItemFilterProxyModel.cpp`: Implementacja klasy.
- `itemList.h`: Potencjalna klasa integrująca model proxy w aplikacji inwentaryzacyjnej.
- [Qt Documentation](https://doc.qt.io): Dokumentacja frameworku Qt.
- [Doxygen](https://www.doxygen.nl): Narzędzie do generowania dokumentacji.