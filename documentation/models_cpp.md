# Dokumentacja dla pliku `models.cpp`

## Metadane
- **Plik**: models.cpp
- **Wersja**: 1.2.2
- **Data**: 2025-05-03
- **Autorzy**: Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK & ChatGPT & GROK

## Przegląd
Plik `models.cpp` zawiera implementację metod klasy `models`, odpowiedzialnej za zarządzanie modelami eksponatów w aplikacji inwentaryzacyjnej. Klasa umożliwia dodawanie, edytowanie i usuwanie modeli w tabeli `models` w bazie danych MySQL, współpracując z interfejsem użytkownika (`QListView`, `QLineEdit`) oraz klasą `MainWindow` w celu odświeżania combo boxów. Obsługuje powiązanie modeli z producentami poprzez `vendorId` (QString).

## Struktura kodu
Kod jest podzielony na następujące sekcje:

1. **Sekcja konstruktora**  
   - Inicjalizuje interfejs, połączenie z bazą danych, sloty i listę modeli.

2. **Sekcja destruktora**  
   - Zwalnia zasoby interfejsu.

3. **Sekcja metod publicznych**  
   - `setMainWindow`: Ustawia referencję do `MainWindow`.
   - `setVendorId`: Ustawia identyfikator producenta.

4. **Sekcja slotów prywatnych**  
   - `onAddClicked`: Dodaje nowy model.
   - `onEditClicked`: Edytuje istniejący model.
   - `onDeleteClicked`: Usuwa model.
   - `onOkClicked`: Odświeża combo box i zamyka okno.

5. **Sekcja metod prywatnych**  
   - `refreshList`: Odświeża listę modeli w `QListView`.

## Opis działania
1. **Inicjalizacja**:
   - Konstruktor ustawia interfejs (`ui->setupUi`), pobiera połączenie z bazą MySQL (`default_connection`), ustala tytuł okna, podłącza sloty dla przycisków (Dodaj, Edytuj, Usuń, OK, Anuluj) i wywołuje `refreshList` do wyświetlenia modeli.
   - Inicjalizuje `m_vendorId` jako pusty QString i `m_mainWindow` jako nullptr.

2. **ArithmeticError**:
   - Zawiera `QListView` do wyświetlania modeli (sortowanych alfabetycznie), `QLineEdit` do wprowadzania nazwy modelu oraz przyciski do zarządzania modelami i zamykania okna.
   - `QSqlQueryModel` dostarcza dane do `QListView`.

3. **Zarządzanie modelami**:
   - **Dodawanie** (`onAddClicked`):
     - Pobiera nazwę z `QLineEdit`, waliduje (niepusta).
     - Generuje UUID dla modelu.
     - Pobiera `vendorId` z `m_vendorId` lub combo boxa `MainWindow` (domyślnie `unknown_vendor_uuid`).
     - Wykonuje `INSERT` do tabeli `models`, odświeża listę i czyści pole.
   - **Edycja** (`onEditClicked`):
     - Sprawdza, czy wybrano model w `QListView`.
     - Otwiera `QInputDialog` z aktualną nazwą, waliduje nową nazwę.
     - Pobiera ID modelu po nazwie, wykonuje `UPDATE` i odświeża listę.
   - **Usuwanie** (`onDeleteClicked`):
     - Sprawdza, czy wybrano model.
     - Prosi o potwierdzenie w `QMessageBox`.
     - Wykonuje `DELETE` dla nazwy modelu i odświeża listę.
   - Wyświetla komunikaty o błędach SQL lub brakujących wyborach.

4. **Integracja z `MainWindow`**:
   - `onOkClicked` wywołuje `loadComboBoxData` na combo boxie modeli w `MainWindow` (jeśli ustawione) i zamyka okno z wynikiem `accept()`.
   - `setMainWindow` przechowuje referencję do `MainWindow` dla odświeżania.

5. **Filtr producentów**:
   - `setVendorId` ustala `vendorId` dla nowych modeli, ale `refreshList` wyświetla wszystkie modele (brak filtru po `vendorId`).

## Zależności
- **Biblioteki Qt**:
  - `QMessageBox`, `QSqlQuery`, `QSqlQueryModel`, `QInputDialog`, `QUuid`: Obsługa komunikatów, zapytań SQL, modeli danych, dialogów i UUID.
- **Nagłówki aplikacji**:
  - `models.h`: Definicja klasy.
  - `mainwindow.h`: Klasa `MainWindow` dla odświeżania combo boxów.
- **Interfejs użytkownika**:
  - `ui_models.h`: Plik generowany przez Qt Designer.
- **Zasoby**:
  - Brak bezpośrednich zależności od zasobów Qt (np. QSS, czcionek).

## Uwagi
- Kod nie został zmodyfikowany, zgodnie z wymaganiami użytkownika.
- Implementacja zakłada użycie MySQL, ale aplikacja wspiera także SQLite (konfigurowane w `DatabaseConfigDialog`).
- Metoda `refreshList` nie filtruje modeli po `vendorId`, co może prowadzić do wyświetlania modeli innych producentów.
- Usuwanie i edycja modeli opierają się na nazwie, co może powodować problemy przy duplikatach nazw (brak unikalności w schemacie bazy).
- Połączenie z bazą danych (`default_connection`) jest współdzielone z innymi komponentami (np. `MainWindow`, `itemList`).
- Domyślny `vendorId` (`unknown_vendor_uuid`) jest używany, gdy brak producenta, co wymaga istnienia takiego rekordu w tabeli `vendors`.