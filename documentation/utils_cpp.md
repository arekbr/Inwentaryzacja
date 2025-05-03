# Dokumentacja dla pliku `utils.cpp`

## Metadane
- **Plik**: utils.cpp
- **Wersja**: 1.2.2
- **Data**: 2025-05-03
- **Autorzy**: Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK & ChatGPT & GROK

## Przegląd
Plik `utils.cpp` zawiera implementację funkcji `setupDatabase`, która konfiguruje i otwiera połączenie z bazą danych SQLite lub MySQL w aplikacji inwentaryzacyjnej. Funkcja rejestruje połączenie pod nazwą "default_connection", używaną przez inne komponenty aplikacji (np. `MainWindow`, `types`, `status`). Dla SQLite automatycznie tworzy schemat bazy danych (tabele i dane początkowe), co zapewnia gotowość bazy do pracy po pierwszym uruchomieniu.

## Struktura kodu
Kod jest podzielony na następujące sekcje:

1. **Sekcja implementacji funkcji `setupDatabase`**  
   - Konfiguruje połączenie, otwiera bazę i obsługuje błędy.
   - Dla SQLite tworzy schemat bazy i przykładowe dane.

2. **Sekcja tworzenia schematu SQLite**  
   - Definiuje tabele: `eksponaty`, `types`, `vendors`, `models`, `statuses`, `storage_places`, `photos`.
   - Wypełnia tabele słownikowe przykładowymi danymi (np. "Komputer", "Atari", "Sprawny").

## Opis działania
1. **Inicjalizacja połączenia**:
   - Usuwa istniejące połączenie "default_connection" (`QSqlDatabase::removeDatabase`).
   - Tworzy nowe połączenie z driverem `QSQLITE` (dla `dbType="SQLite3"`) lub `QMYSQL` (dla `dbType="MySQL"`).
   - Konfiguruje parametry:
     - **MySQL**: Ustawia `host`, `dbSource` (nazwa bazy), `user`, `password`, `port`.
     - **SQLite**: Ustawia `dbSource` jako ścieżkę do pliku bazy.
   - Próbuje otworzyć połączenie, wyświetlając `QMessageBox` z błędem w przypadku niepowodzenia.

2. **Tworzenie schematu dla SQLite**:
   - Sprawdza, czy wszystkie wymagane tabele (`eksponaty`, `types`, `vendors`, `models`, `statuses`, `storage_places`, `photos`) istnieją.
   - Jeśli brakuje którejkolwiek tabeli, wykonuje zapytania SQL:
     - **Tabela `eksponaty`**: Przechowuje dane eksponatów (np. nazwa, typ, producent, status).
     - **Tabele słownikowe** (`types`, `vendors`, `models`, `statuses`, `storage_places`): Przechowują unikalne nazwy z UUID jako kluczem.
     - **Tabela `photos`**: Przechowuje zdjęcia powiązane z eksponatami (BLOB).
   - Wypełnia tabele słownikowe przykładowymi danymi:
     - Typy: "Komputer", "Monitor", "Kabel".
     - Producenci: "Atari", "Commodore", "Sinclair".
     - Modele: "Atari 800XL", "Amiga 500", "ZX Spectrum".
     - Statusy: "Sprawny", "Uszkodzony", "W naprawie".
     - Miejsce przechowywania: "Magazyn 1", "Półka B3".
   - Używa `QUuid` do generowania unikalnych identyfikatorów.

3. **Zwracana wartość**:
   - Zwraca `true` po pomyślnym nawiązaniu połączenia i (dla SQLite) utworzeniu schematu.
   - Zwraca `false` w przypadku błędu połączenia.

## Zależności
- **Biblioteki Qt**:
  - `QMessageBox`: Wyświetlanie błędów połączenia.
  - `QSqlDatabase`, `QSqlError`, `QSqlQuery`: Obsługa połączenia i zapytań SQL.
  - `QUuid`: Generowanie unikalnych identyfikatorów.
- **Nagłówki aplikacji**:
  - Brak zależności od innych nagłówków aplikacji (tylko Qt).
- **Zasoby**:
  - Brak bezpośrednich zależności od zasobów Qt (np. QSS, czcionek).

## Uwagi
- Kod nie został zmodyfikowany, zgodnie z wymaganiami użytkownika.
- Funkcja `setupDatabase` jest centralnym elementem konfiguracji bazy danych, używanym przez inne komponenty aplikacji.
- Dla SQLite automatyczne tworzenie schematu i danych początkowych zapewnia gotowość bazy, ale brak podobnego mechanizmu dla MySQL (zakłada, że schemat już istnieje).
- Schemat bazy używa UUID jako kluczy głównych, co zapewnia unikalność, ale operacje CRUD w innych klasach (np. `types`, `status`) opierają się na nazwach, co może powodować problemy przy duplikatach.
- Połączenie "default_connection" jest współdzielone, co wymaga ostrożnego zarządzania w przypadku wielokrotnego wywoływania funkcji.
- Przykładowe dane są zorientowane na retro sprzęt komputerowy, co odzwierciedla kontekst aplikacji.