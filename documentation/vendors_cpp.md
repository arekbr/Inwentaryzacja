# Dokumentacja dla pliku `vendors.cpp`

## Metadane
- **Plik**: vendors.cpp
- **Wersja**: 1.2.2
- **Data**: 2025-05-03
- **Autorzy**: Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK & ChatGPT & GROK

## Przegląd
Plik `vendors.cpp` zawiera implementację metod klasy `vendors`, odpowiedzialnej za zarządzanie producentami sprzętu (np. Commodore, IBM, Apple) w aplikacji inwentaryzacyjnej. Klasa umożliwia dodawanie, edytowanie i usuwanie producentów w tabeli `vendors` w bazie danych MySQL, współpracując z interfejsem użytkownika (`QListView`, `QLineEdit`) oraz klasą `MainWindow` w celu odświeżania combo boxów producentów po zapisaniu zmian.

## Struktura kodu
Kod jest podzielony na następujące sekcje:

1. **Sekcja konstruktora**  
   - Inicjalizuje interfejs, połączenie z bazą danych, sloty i listę producentów.

2. **Sekcja destruktora**  
   - Zwalnia zasoby interfejsu.

3. **Sekcja metod publicznych**  
   - `setMainWindow`: Ustawia referencję do `MainWindow`.

4. **Sekcja slotów prywatnych**  
   - `onAddClicked`: Dodaje nowego producenta.
   - `onEditClicked`: Edytuje istniejącego producenta.
   - `onDeleteClicked`: Usuwa producenta.
   - `onOkClicked`: Odświeża combo box i zamyka okno.

5. **Sekcja metod prywatnych**  
   - `refreshList`: Odświeża listę producentów w `QListView`.

## Opis działania
1. **Inicjalizacja**:
   - Konstruktor ustawia interfejs (`ui->setupUi`), pobiera połączenie z bazą MySQL (`default_connection`), ustala tytuł okna, podłącza sloty dla przycisków (Dodaj, Edytuj, Usuń, OK, Anuluj) i wywołuje `refreshList` do wyświetlenia producentów.
   - Inicjalizuje `m_mainWindow` jako `nullptr`.

2. **Interfejs użytkownika**:
   - Zawiera `QListView` do wyświetlania producentów (sortowanych alfabetycznie), `QLineEdit` do wprowadzania nazwy producenta oraz przyciski do zarządzania producentami i zamykania okna.
   - `QSqlQueryModel` dostarcza dane do `QListView`.

3. **Zarządzanie producentami**:
   - **Dodawanie** (`onAddClicked`):
     - Pobiera nazwę z `QLineEdit`, waliduje (niepusta).
     - Generuje UUID dla producenta i zapisuje do zmiennej `newId`.
     - Wykonuje `INSERT` do tabeli `vendors`, odświeża listę i czyści pole.
   - **Edycja** (`onEditClicked`):
     - Sprawdza, czy wybrano producenta w `QListView`.
     - Otwiera `QInputDialog` z aktualną nazwą, waliduje nową nazwę.
     - Pobiera ID producenta po nazwie, wykonuje `UPDATE` i odświeża listę.
   - **Usuwanie** (`onDeleteClicked`):
     - Sprawdza, czy wybrano producenta.
     - Prosi o potwierdzenie w `QMessageBox`.
     - Wykonuje `DELETE` dla nazwy producenta i odświeża listę.
   - Wyświetla komunikaty o błędach SQL lub brakujących wyborach.

4. **Integracja z `MainWindow`**:
   - `onOkClicked` wywołuje `loadComboBoxData` na combo boxie producentów w `MainWindow` (jeśli ustawione) i zamyka okno z wynikiem `accept()`.
   - `setMainWindow` przechowuje referencję do `MainWindow` dla odświeżania.

## Zależności
- **Biblioteki Qt**:
  - `QMessageBox`, `QSqlQuery`, `QSqlQueryModel`, `QInputDialog`, `QUuid`: Obsługa komunikatów, zapytań SQL, modeli danych, dialogów i UUID.
- **Nagłówki aplikacji**:
  - `vendors.h`: Definicja klasy.
  - `mainwindow.h`: Klasa `MainWindow` dla odświeżania combo boxów.
- **Interfejs użytkownika**:
  - `ui_vendors.h`: Plik generowany przez Qt Designer.
- **Zasoby**:
  - Brak bezpośrednich zależności od zasobów Qt (np. QSS, czcionek).

## Uwagi
- Kod nie został zmodyfikowany, zgodnie z wymaganiami użytkownika.
- Implementacja zakłada użycie MySQL, ale aplikacja wspiera także SQLite (konfigurowane w `DatabaseConfigDialog`).
- Usuwanie i edycja producentów opierają się na nazwie, co może powodować problemy przy duplikatach nazw (brak unikalności w schemacie bazy).
- Połączenie z bazą danych (`default_connection`) jest współdzielone z innymi komponentami (np. `MainWindow`, `models`).
- Lista producentów w `QListView` nie jest filtrowana (np. po kategorii), co może być rozszerzone w przyszłości.
- W przeciwieństwie do niektórych innych klas słownikowych (np. `storage`), metoda `onAddClicked` poprawnie przechowuje UUID w zmiennej, co eliminuje problem nieużywanego generowania UUID.