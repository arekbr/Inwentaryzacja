# Dokumentacja dla pliku `models.h`

## Metadane
- **Plik**: models.h
- **Wersja**: 1.2.2
- **Data**: 2025-05-03
- **Autorzy**: Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK & ChatGPT & GROK

## Przegląd
Plik `models.h` definiuje klasę `models`, która reprezentuje okno dialogowe do zarządzania modelami eksponatów w aplikacji inwentaryzacyjnej. Klasa umożliwia dodawanie, edytowanie i usuwanie modeli w tabeli `models` w bazie danych, a także odświeżanie listy modeli w combo boxie głównego okna (`MainWindow`). Obsługuje identyfikator producenta (`vendorId`) jako `QString`, co pozwala powiązać modele z konkretnymi producentami.

## Struktura kodu
Plik nagłówkowy zawiera następujące elementy:

1. **Deklaracja klasy `models`**  
   - Dziedziczy po `QDialog`.
   - Używa makra `Q_OBJECT` dla wsparcia mechanizmu sygnałów i slotów Qt.

2. **Metody publiczne**  
   - Konstruktor: Inicjalizuje okno dialogowe, interfejs i połączenie z bazą danych.
   - Destruktor: Zwalnia zasoby.
   - `setMainWindow`: Ustawia referencję do `MainWindow` dla odświeżania combo boxów.
   - `setVendorId`: Ustawia identyfikator producenta dla nowych modeli.

3. **Sloty prywatne**  
   - `onAddClicked`: Dodaje nowy model do tabeli `models`.
   - `onEditClicked`: Edytuje nazwę istniejącego modelu.
   - `onDeleteClicked`: Usuwa wybrany model po potwierdzeniu.
   - `onOkClicked`: Odświeża combo box w `MainWindow` i zamyka okno.

4. **Metody prywatne**  
   - `refreshList`: Odświeża listę modeli w `QListView`.

5. **Zmienne prywatne**  
   - `ui`: Wskaźnik na interfejs użytkownika.
   - `m_mainWindow`: Wskaźnik na główne okno aplikacji.
   - `m_db`: Połączenie z bazą danych MySQL.
   - `m_vendorId`: Identyfikator producenta (UUID jako QString).

## Opis działania
- **Inicjalizacja**: Konstruktor konfiguruje interfejs (`ui->setupUi`), ustala połączenie z bazą danych (`default_connection`), ustawia tytuł okna ("Zarządzanie modelami sprzętu"), podłącza sloty dla przycisków i odświeża listę modeli.
- **Interfejs**: Zawiera `QListView` do wyświetlania modeli, `QLineEdit` do wprowadzania nazwy modelu oraz przyciski (Dodaj, Edytuj, Usuń, OK, Anuluj).
- **Zarządzanie modelami**:
  - **Dodawanie**: Pobiera nazwę z `QLineEdit`, generuje UUID, zapisuje model z powiązaniem do producenta (`vendorId` lub z `MainWindow`).
  - **Edycja**: Otwiera `QInputDialog` dla wybranej nazwy modelu, aktualizuje rekord w bazie.
  - **Usuwanie**: Usuwa model po potwierdzeniu, wykonując zapytanie `DELETE`.
- **Integracja z `MainWindow`**: Po zatwierdzeniu zmian (`onOkClicked`), odświeża combo box modeli w głównym oknie, jeśli `m_mainWindow` jest ustawione.
- **Filtr producentów**: `setVendorId` pozwala ograniczyć modele do wybranego producenta, choć `refreshList` obecnie nie filtruje listy.

## Zależności
- **Biblioteki Qt**:
  - `QDialog`, `QSqlDatabase`: Obsługa okna dialogowego i bazy danych.
- **Nagłówki aplikacji**:
  - `mainwindow.h`: Deklaracja klasy `MainWindow` dla komunikacji.
- **Interfejs użytkownika**:
  - `ui_models.h`: Plik generowany przez Qt Designer.
- **Zasoby**:
  - Brak bezpośrednich zależności od zasobów Qt (np. QSS, czcionek).

## Uwagi
- Kod nie został zmodyfikowany, zgodnie z wymaganiami użytkownika.
- Klasa jest częścią systemu słownikowego, wspiera zarządzanie modelami w kontekście producentów.
- Metoda `refreshList` nie filtruje modeli po `vendorId`, co może być rozszerzone w przyszłości.
- Połączenie z bazą danych (`default_connection`) jest współdzielone z innymi komponentami (np. `MainWindow`, `itemList`).
- Modele są zapisywane z domyślnym `vendorId` (`unknown_vendor_uuid`), jeśli nie wybrano producenta.