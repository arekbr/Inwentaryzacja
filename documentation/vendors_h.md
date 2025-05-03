# Dokumentacja dla pliku `vendors.h`

## Metadane
- **Plik**: vendors.h
- **Wersja**: 1.2.2
- **Data**: 2025-05-03
- **Autorzy**: Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK & ChatGPT & GROK

## Przegląd
Plik `vendors.h` definiuje klasę `vendors`, która reprezentuje okno dialogowe do zarządzania producentami sprzętu (np. Commodore, IBM, Apple) w aplikacji inwentaryzacyjnej. Klasa umożliwia dodawanie, edytowanie i usuwanie producentów w tabeli `vendors` w bazie danych oraz synchronizację z głównym oknem (`MainWindow`) w celu odświeżenia combo boxów producentów po zapisaniu zmian. Jest częścią systemu słownikowego aplikacji, podobnie jak klasy `models`, `status`, `storage` czy `types`.

## Struktura kodu
Plik nagłówkowy zawiera następujące elementy:

1. **Deklaracja klasy `vendors`**  
   - Dziedziczy po `QDialog`.
   - Używa makra `Q_OBJECT` dla wsparcia mechanizmu sygnałów i slotów Qt.

2. **Metody publiczne**  
   - Konstruktor: Inicjalizuje okno dialogowe, interfejs i połączenie z bazą danych.
   - Destruktor: Zwalnia zasoby.
   - `setMainWindow`: Ustawia referencję do `MainWindow` dla odświeżania combo boxów.

3. **Sloty prywatne**  
   - `onAddClicked`: Dodaje nowego producenta do tabeli `vendors`.
   - `onEditClicked`: Edytuje nazwę istniejącego producenta.
   - `onDeleteClicked`: Usuwa wybranego producenta po potwierdzeniu.
   - `onOkClicked`: Odświeża combo box w `MainWindow` i zamyka okno.

4. **Metody prywatne**  
   - `refreshList`: Odświeża listę producentów w `QListView`.

5. **Zmienne prywatne**  
   - `ui`: Wskaźnik na interfejs użytkownika.
   - `m_mainWindow`: Wskaźnik na główne okno aplikacji.
   - `m_db`: Połączenie z bazą danych MySQL.

## Opis działania
- **Inicjalizacja**: Konstruktor konfiguruje interfejs (`ui->setupUi`), ustala połączenie z bazą danych (`default_connection`), ustawia tytuł okna ("Zarządzanie producentami"), podłącza sloty dla przycisków i odświeża listę producentów.
- **Interfejs**: Zawiera `QListView` do wyświetlania producentów, `QLineEdit` do wprowadzania nazwy producenta oraz przyciski (Dodaj, Edytuj, Usuń, OK, Anuluj).
- **Zarządzanie producentami**:
  - **Dodawanie**: Pobiera nazwę z `QLineEdit`, generuje UUID, zapisuje producenta w tabeli `vendors`.
  - **Edycja**: Otwiera `QInputDialog` dla wybranej nazwy producenta, aktualizuje rekord w bazie.
  - **Usuwanie**: Usuwa producenta po potwierdzeniu, wykonując zapytanie `DELETE`.
- **Integracja z `MainWindow`**: Po zatwierdzeniu zmian (`onOkClicked`), odświeża combo box producentów w głównym oknie, jeśli `m_mainWindow` jest ustawione.
- **Baza danych**: Operuje na tabeli `vendors` z polami `id` (UUID) i `name` (nazwa producenta).

## Zależności
- **Biblioteki Qt**:
  - `QDialog`, `QSqlDatabase`: Obsługa okna dialogowego i bazy danych.
- **Nagłówki aplikacji**:
  - `mainwindow.h`: Deklaracja klasy `MainWindow` dla komunikacji.
- **Interfejs użytkownika**:
  - `ui_vendors.h`: Plik generowany przez Qt Designer.
- **Zasoby**:
  - Brak bezpośrednich zależności od zasobów Qt (np. QSS, czcionek).

## Uwagi
- Kod nie został zmodyfikowany, zgodnie z wymaganiami użytkownika.
- Klasa jest częścią systemu słownikowego, wspiera zarządzanie producentami w kontekście eksponatów.
- Połączenie z bazą danych (`default_connection`) jest współdzielone z innymi komponentami (np. `MainWindow`, `models`).
- Operacje CRUD (Create, Read, Update, Delete) opierają się na nazwie producenta, co może powodować problemy przy duplikatach (brak unikalności w schemacie bazy).
- Klasa nie filtruje producentów (np. po kategorii), co może być rozszerzone w przyszłości.