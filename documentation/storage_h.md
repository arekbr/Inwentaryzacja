# Dokumentacja dla pliku `storage.h`

## Metadane
- **Plik**: storage.h
- **Wersja**: 1.2.2
- **Data**: 2025-05-03
- **Autorzy**: Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK & ChatGPT & GROK

## Przegląd
Plik `storage.h` definiuje klasę `storage`, która reprezentuje okno dialogowe do zarządzania miejscami przechowywania eksponatów (np. magazyny, półki, pokoje) w aplikacji inwentaryzacyjnej. Klasa umożliwia dodawanie, edytowanie i usuwanie lokalizacji w tabeli `storage_places` w bazie danych oraz synchronizację z głównym oknem (`MainWindow`) w celu odświeżenia combo boxów lokalizacji po zapisaniu zmian. Jest częścią systemu słownikowego aplikacji, podobnie jak klasy `models`, `status` czy `vendors`.

## Struktura kodu
Plik nagłówkowy zawiera następujące elementy:

1. **Deklaracja klasy `storage`**  
   - Dziedziczy po `QDialog`.
   - Używa makra `Q_OBJECT` dla wsparcia mechanizmu sygnałów i slotów Qt.

2. **Metody publiczne**  
   - Konstruktor: Inicjalizuje okno dialogowe, interfejs i połączenie z bazą danych.
   - Destruktor: Zwalnia zasoby.
   - `setMainWindow`: Ustawia referencję do `MainWindow` dla odświeżania combo boxów.

3. **Sloty prywatne**  
   - `onAddClicked`: Dodaje nową lokalizację do tabeli `storage_places`.
   - `onEditClicked`: Edytuje nazwę istniejącej lokalizacji.
   - `onDeleteClicked`: Usuwa wybraną lokalizację po potwierdzeniu.
   - `onOkClicked`: Odświeża combo box w `MainWindow` i zamyka okno.

4. **Metody prywatne**  
   - `refreshList`: Odświeża listę lokalizacji w `QListView`.

5. **Zmienne prywatne**  
   - `ui`: Wskaźnik na interfejs użytkownika.
   - `m_mainWindow`: Wskaźnik na główne okno aplikacji.
   - `m_db`: Połączenie z bazą danych MySQL.

## Opis działania
- **Inicjalizacja**: Konstruktor konfiguruje interfejs (`ui->setupUi`), ustala połączenie z bazą danych (`default_connection`), ustawia tytuł okna ("Zarządzanie miejscami przechowywania"), podłącza sloty dla przycisków i odświeża listę lokalizacji.
- **Interfejs**: Zawiera `QListView` do wyświetlania lokalizacji, `QLineEdit` do wprowadzania nazwy lokalizacji oraz przyciski (Dodaj, Edytuj, Usuń, OK, Anuluj).
- **Zarządzanie lokalizacjami**:
  - **Dodawanie**: Pobiera nazwę z `QLineEdit`, generuje UUID, zapisuje lokalizację w tabeli `storage_places`.
  - **Edycja**: Otwiera `QInputDialog` dla wybranej nazwy lokalizacji, aktualizuje rekord w bazie.
  - **Usuwanie**: Usuwa lokalizację po potwierdzeniu, wykonując zapytanie `DELETE`.
- **Integracja z `MainWindow`**: Po zatwierdzeniu zmian (`onOkClicked`), odświeża combo box lokalizacji w głównym oknie, jeśli `m_mainWindow` jest ustawione.
- **Baza danych**: Operuje na tabeli `storage_places` z polami `id` (UUID) i `name` (nazwa lokalizacji).

## Zależności
- **Biblioteki Qt**:
  - `QDialog`, `QSqlDatabase`: Obsługa okna dialogowego i bazy danych.
- **Nagłówki aplikacji**:
  - `mainwindow.h`: Deklaracja klasy `MainWindow` dla komunikacji.
- **Interfejs użytkownika**:
  - `ui_storage.h`: Plik generowany przez Qt Designer.
- **Zasoby**:
  - Brak bezpośrednich zależności od zasobów Qt (np. QSS, czcionek).

## Uwagi
- Kod nie został zmodyfikowany, zgodnie z wymaganiami użytkownika.
- Klasa jest częścią systemu słownikowego, wspiera zarządzanie lokalizacjami w kontekście eksponatów.
- Połączenie z bazą danych (`default_connection`) jest współdzielone z innymi komponentami (np. `MainWindow`, `models`).
- Operacje CRUD (Create, Read, Update, Delete) opierają się na nazwie lokalizacji, co może powodować problemy przy duplikatach (brak unikalności w schemacie bazy).
- Klasa nie filtruje lokalizacji (np. po typie magazynu), co może być rozszerzone w przyszłości.