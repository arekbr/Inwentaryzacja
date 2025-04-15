# Dziennik zmian

Wszystkie istotne zmiany w projekcie Inwentaryzacja będą dokumentowane w tym pliku.

Format opiera się na [Keep a Changelog](https://keepachangelog.com/pl/1.0.0/), a projekt stosuje [Wersjonowanie Semantyczne](https://semver.org/lang/pl/).

## [1.2.0] - 2025-04-15

### Dodano
- **Konfiguracja bazy danych**: Nowe okno (`DatabaseConfigDialog`) do wyboru i ustawiania bazy danych:
  - Obsługa SQLite3 (wybór pliku za pomocą `QFileDialog`).
  - Obsługa MySQL (ustawienia hosta, portu, użytkownika, hasła, nazwy bazy).
  - Zapisywanie ustawień w `QSettings` dla trwałości konfiguracji.
- **Zarządzanie statusami**: Dedykowane okno (`status.ui`) z pełnymi operacjami:
  - Dodawanie, edycja i usuwanie statusów w bazie danych.
  - Integracja z głównym oknem (odświeżanie listy statusów w `MainWindow`).
  - Użycie UUID dla unikalnych identyfikatorów.
- **Zarządzanie miejscami przechowywania**: Nowe okno (`storage.ui`) do obsługi lokalizacji:
  - Dodawanie, edycja i usuwanie miejsc przechowywania.
  - Synchronizacja z głównym oknem aplikacji.
  - UUID dla identyfikatorów.
- **Obsługa zdjęć**: Nowe moduły (`photoitem`, `fullscreenphotoviewer`) umożliwiające:
  - Dodawanie zdjęć do elementów inwentaryzacji.
  - Przeglądanie zdjęć w trybie pełnoekranowym.
  - Zarządzanie zdjęciami w bazie danych.
- **Ulepszone zarządzanie typami i producentami**:
  - Przejście na okna dialogowe (`QDialog`) w `types.cpp` i `vendors.cpp`.
  - Pełne operacje CRUD (tworzenie, odczyt, aktualizacja, usuwanie).
  - Interfejsy z listami (`QListView`) i przyciskami akcji (dodaj, edytuj, usuń).
  - Synchronizacja z głównym oknem dla dynamicznych list rozwijanych.
- **Funkcja konfiguracji bazy**: Nowy moduł `utils.cpp` z funkcją `setupDatabase`:
  - Ujednolicona konfiguracja połączenia dla SQLite3 i MySQL.
  - Obsługa błędów z komunikatami w `QMessageBox`.
- **Skrypty SQL**:
  - `Create_MySQL.sql`: Inicjalizacja bazy MySQL z pełną strukturą tabel.
  - `createTable.sql`: Skrypt do tworzenia tabel dla nowych funkcji.
  - `migrate_to_uuid.sql`: Migracja identyfikatorów na UUID dla istniejących danych.
  - `database_example_record.sql`: Przykładowe rekordy do testowania bazy.
- **Dokumentacja**: Plik `README.md` opisujący:
  - Cel projektu (inwentaryzacja retro komputerów).
  - Proces tworzenia z użyciem AI (ChatGPT, GROK, DeepSeek).
  - Wymagania (Qt 6.8.1, SQLite3) i instrukcje instalacji.
- **Gitignore**: Plik `.gitignore` ignorujący pliki tymczasowe (`build/`, `*.db`, `*.DS_Store` itp.).

### Zmieniono
- **Główne okno** (`mainwindow.cpp`, `mainwindow.ui`):
  - Dodano zaawansowane filtrowanie i wyszukiwanie elementów w liście inwentaryzacyjnej.
  - Wprowadzono obsługę UUID dla nowych rekordów.
  - Zintegrowano nowe moduły (statusy, miejsca przechowywania, zdjęcia).
  - Ulepszono zarządzanie danymi w tabelach (`QTableView`) z dynamicznym ładowaniem.
- **Lista elementów** (`itemList.cpp`):
  - Rozbudowano o możliwość edycji i usuwania bezpośrednio z interfejsu.
  - Poprawiono integrację z bazą danych dla nowych tabel (statusy, miejsca).
- **Modele** (`models.cpp`):
  - Zaktualizowano o obsługę nowych typów danych (np. zdjęcia, statusy).
  - Ulepszono ładowanie i zapisywanie danych z bazy.
- **Interfejsy typów i producentów** (`types.ui`, `vendors.ui`):
  - Dodano widoki list (`QListView`) dla przeglądania rekordów.
  - Wprowadzono przyciski akcji (dodaj, edytuj, usuń) dla łatwiejszej obsługi.
  - Zwiększono szerokość okien dla lepszej czytelności (np. z 400px na 546px dla `types.ui`).
- **Plik projektu** (`Inwentaryzacja.pro`):
  - Dodano moduł `widgets` dla pełnej obsługi Qt Widgets.
  - Włączono nowe pliki źródłowe, nagłówki, formularze i skrypty SQL.
  - Ulepszono konfigurację tłumaczeń (`lrelease`, `embed_translations`) dla `Inwentaryzacja_pl_PL.ts`.
- **Struktura bazy danych**:
  - Przejście na UUID dla tabel (`types`, `vendors`, `statuses`, `storage_places`).
  - Rozbudowa schematu o nowe tabele dla statusów i miejsc przechowywania.
- **Kod i organizacja**:
  - Przepisano `types` i `vendors` z `QWidget` na `QDialog` dla lepszej modalności.
  - Dodano zarządzanie błędami w interakcjach z bazą (np. komunikaty w `QMessageBox`).
  - Usprawniono połączenia sygnałów-slotów w nowych oknach.

### Poprawiono
- Stabilność interfejsu użytkownika dzięki lepszemu zarządzaniu danymi w `QSqlQueryModel`.
- Poprawiono obsługę błędów bazy danych w nowych modułach (`status`, `storage`, `types`, `vendors`).
- Zoptymalizowano ładowanie danych w głównym oknie dla większych zbiorów.
- Ujednolicono styl interfejsu w nowych oknach (np. spójne rozmiary przycisków i pól).

## [1.0.0] - 11 marca 2025
### Dodano
- Początkowe wydanie aplikacji Inwentaryzacja do zarządzania kolekcją retro komputerów.
- Podstawowe funkcje: przeglądanie listy elementów, zarządzanie typami, producentami i modelami.
- Obsługa bazy danych SQLite3 do przechowywania danych.
- Prosty interfejs użytkownika z głównym oknem, listą elementów i formularzami.
