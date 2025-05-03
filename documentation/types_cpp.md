# Dokumentacja dla pliku `types.cpp`

## Metadane
- **Plik**: types.cpp
- **Wersja**: 1.2.2
- **Data**: 2025-05-03
- **Autorzy**: Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK & ChatGPT & GROK

## Przegląd
Plik `types.cpp` zawiera implementację metod klasy `types`, odpowiedzialnej za zarządzanie typami eksponatów (np. Komputer, Monitor, Drukarka) w aplikacji inwentaryzacyjnej. Klasa umożliwia dodawanie, edytowanie i usuwanie typów w tabeli `types` w bazie danych MySQL, współpracując z interfejsem użytkownika (`QListView`, `QLineEdit`) oraz klasą `MainWindow` w celu odświeżania combo boxów typów po zapisaniu zmian.

## Struktura kodu
Kod jest podzielony na następujące sekcje:

1. **Sekcja konstruktora**  
   - Inicjalizuje interfejs, połączenie z bazą danych, sloty i listę typów.

2. **Sekcja destruktora**  
   - Zwalnia zasoby interfejsu.

3. **Sekcja metod publicznych**  
   - `setMainWindow`: Ustawia referencję do `MainWindow`.

4. **Sekcja slotów prywatnych**  
   - `onAddClicked`: Dodaje nowy typ.
   - `onEditClicked`: Edytuje istniejący typ.
   - `onDeleteClicked`: Usuwa typ.
   - `onOkClicked`: Odświeża combo box i zamyka okno.

5. **Sekcja metod prywatnych**  
   - `refreshList`: Odświeża listę typów w `QListView`.

## Opis działania
1. **Inicjalizacja**:
   - Konstruktor ustawia interfejs (`ui->setupUi`), pobiera połączenie z bazą MySQL (`default_connection`), ustala tytuł okna, podłącza sloty dla przycisków (Dodaj, Edytuj, Usuń, OK, Anuluj) i wywołuje `refreshList` do wyświetlenia typów.
   - Inicjalizuje `m_mainWindow` jako `nullptr`.

2. **Interfejs użytkownika**:
   - Zawiera `QListView` do wyświetlania typów (sortowanych alfabetycznie), `QLineEdit` (nazwane `type_lineEdit`) do wprowadzania nazwy typu oraz przyciski do zarządzania typami i zamykania okna.
   - `QSqlQueryModel` dostarcza dane do `QListView`.

3. **Zarządzanie typami**:
   - **Dodawanie** (`onAddClicked`):
     - Pobiera nazwę z `type_lineEdit`, waliduje (niepusta).
     - Generuje UUID dla typu i zapisuje do zmiennej `uuid`.
     - Wykonuje `INSERT` do tabeli `types`, odświeża listę i czyści pole.
   - **Edycja** (`onEditClicked`):
     - Sprawdza, czy wybrano typ w `QListView`.
     - Otwiera `QInputDialog` z aktualną nazwą, waliduje nową nazwę.
     - Pobiera ID typu po nazwie, wykonuje `UPDATE` i odświeża listę.
   - **Usuwanie** (`onDeleteClicked`):
     - Sprawdza, czy wybrano typ.
     - Prosi o potwierdzenie w `QMessageBox`.
     - Wykonuje `DELETE` dla nazwy typu i odświeża listę.
   - Wyświetla komunikaty o błędach SQL lub brakujących wyborach.

4. **Integracja z `MainWindow`**:
   - `onOkClicked` wywołuje `loadComboBoxData` na combo boxie typów w `MainWindow` (jeśli ustawione) i zamyka okno z wynikiem `accept()`.
   - `setMainWindow` przechowuje referencję do `MainWindow` dla odświeżania.

## Zależności
- **Biblioteki Qt**:
  - `QMessageBox`, `QSqlQuery`, `QSqlQueryModel`, `QInputDialog`, `QUuid`: Obsługa komunikatów, zapytań SQL, modeli danych, dialogów i UUID.
- **Nagłówki aplikacji**:
  - `types.h`: Definicja klasy.
  - `mainwindow.h`: Klasa `MainWindow` dla odświeżania combo boxów.
- **Interfejs użytkownika**:
  - `ui_types.h`: Plik generowany przez Qt Designer.
- **Zasoby**:
  - Brak bezpośrednich zależności od zasobów Qt (np. QSS, czcionek).

## Uwagi
- Kod nie został zmodyfikowany, zgodnie z wymaganiami użytkownika.
- Implementacja zakłada użycie MySQL, ale aplikacja wspiera także SQLite (konfigurowane w `DatabaseConfigDialog`).
- Usuwanie i edycja typów opierają się na nazwie, co może powodować problemy przy duplikatach nazw (brak unikalności w schemacie bazy).
- Połączenie z bazą danych (`default_connection`) jest współdzielone z innymi komponentami (np. `MainWindow`, `models`).
- Lista typów w `QListView` nie jest filtrowana (np. po kategorii), co może być rozszerzone w przyszłości.
- W przeciwieństwie do innych klas słownikowych (np. `status`, `storage`), metoda `onAddClicked` poprawnie przechowuje UUID w zmiennej, co eliminuje problem nieużywanego generowania UUID.