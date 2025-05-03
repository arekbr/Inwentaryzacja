# Dokumentacja dla pliku `status.h`

## Metadane
- **Plik**: status.h
- **Wersja**: 1.2.2
- **Data**: 2025-05-03
- **Autorzy**: Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK & ChatGPT & GROK

## Przegląd
Plik `status.h` definiuje klasę `status`, która reprezentuje okno dialogowe do zarządzania statusami eksponatów (np. "Nowy", "Używany", "Uszkodzony") w aplikacji inwentaryzacyjnej. Klasa umożliwia dodawanie, edytowanie i usuwanie statusów w tabeli `statuses` w bazie danych oraz synchronizację z głównym oknem (`MainWindow`) w celu odświeżenia combo boxów statusów po zapisaniu zmian. Jest częścią systemu słownikowego aplikacji, podobnie jak klasy `models` czy `vendors`.

## Struktura kodu
Plik nagłówkowy zawiera następujące elementy:

1. **Deklaracja klasy `status`**  
   - Dziedziczy po `QDialog`.
   - Używa makra `Q_OBJECT` dla wsparcia mechanizmu sygnałów i slotów Qt.

2. **Metody publiczne**  
   - Konstruktor: Inicjalizuje okno dialogowe, interfejs i połączenie z bazą danych.
   - Destruktor: Zwalnia zasoby.
   - `setMainWindow`: Ustawia referencję do `MainWindow` dla odświeżania combo boxów.

3. **Sloty prywatne**  
   - `onAddClicked`: Dodaje nowy status do tabeli `statuses`.
   - `onEditClicked`: Edytuje nazwę istniejącego statusu.
   - `onDeleteClicked`: Usuwa wybrany status po potwierdzeniu.
   - `onOkClicked`: Odświeża combo box w `MainWindow` i zamyka okno.

4. **Metody prywatne**  
   - `refreshList`: Odświeża listę statusów w `QListView`.

5. **Zmienne prywatne**  
   - `ui`: Wskaźnik na interfejs użytkownika.
   - `m_mainWindow`: Wskaźnik na główne okno aplikacji.
   - `m_db`: Połączenie z bazą danych MySQL.

## Opis działania
- **Inicjalizacja**: Konstruktor konfiguruje interfejs (`ui->setupUi`), ustala połączenie z bazą danych (`default_connection`), ustawia tytuł okna ("Zarządzanie statusami"), podłącza sloty dla przycisków i odświeża listę statusów.
- **Interfejs**: Zawiera `QListView` do wyświetlania statusów, `QLineEdit` do wprowadzania nazwy statusu oraz przyciski (Dodaj, Edytuj, Usuń, OK, Anuluj).
- **Zarządzanie statusami**:
  - **Dodawanie**: Pobiera nazwę z `QLineEdit`, generuje UUID, zapisuje status w tabeli `statuses`.
  - **Edycja**: Otwiera `QInputDialog` dla wybranej nazwy statusu, aktualizuje rekord w bazie.
  - **Usuwanie**: Usuwa status po potwierdzeniu, wykonując zapytanie `DELETE`.
- **Integracja z `MainWindow`**: Po zatwierdzeniu zmian (`onOkClicked`), odświeża combo box statusów w głównym oknie, jeśli `m_mainWindow` jest ustawione.
- **Baza danych**: Operuje na tabeli `statuses` z polami `id` (UUID) i `name` (nazwa statusu).

## Zależności
- **Biblioteki Qt**:
  - `QDialog`, `QSqlDatabase`: Obsługa okna dialogowego i bazy danych.
- **Nagłówki aplikacji**:
  - `mainwindow.h`: Deklaracja klasy `MainWindow` dla komunikacji.
- **Interfejs użytkownika**:
  - `ui_status.h`: Plik generowany przez Qt Designer.
- **Zasoby**:
  - Brak bezpośrednich zależności od zasobów Qt (np. QSS, czcionek).

## Uwagi
- Kod nie został zmodyfikowany, zgodnie z wymaganiami użytkownika.
- Klasa jest częścią systemu słownikowego, wspiera zarządzanie statusami w kontekście eksponatów.
- Połączenie z bazą danych (`default_connection`) jest współdzielone z innymi komponentami (np. `MainWindow`, `models`).
- Operacje CRUD (Create, Read, Update, Delete) opierają się na nazwie statusu, co może powodować problemy przy duplikatach (brak unikalności w schemacie bazy).
- Klasa nie filtruje statusów (np. po kategorii), co może być rozszerzone w przyszłości.