# Dokumentacja dla pliku `utils.h`

## Metadane
- **Plik**: utils.h
- **Wersja**: 1.2.2
- **Data**: 2025-05-03
- **Autorzy**: Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK & ChatGPT & GROK

## Przegląd
Plik `utils.h` definiuje funkcję `setupDatabase`, która jest odpowiedzialna za inicjalizację połączenia z bazą danych w aplikacji inwentaryzacyjnej. Funkcja umożliwia konfigurację połączenia dla baz SQLite (lokalny plik) lub MySQL (zdalne połączenie), rejestrując je pod nazwą "default_connection". Jest to kluczowy komponent aplikacji, wykorzystywany na etapie uruchamiania, np. przez `DatabaseConfigDialog`, do przygotowania bazy danych dla innych komponentów, takich jak `MainWindow`, `types`, `status`, `storage` czy `models`.

## Struktura kodu
Plik nagłówkowy zawiera następujące elementy:

1. **Deklaracja funkcji `setupDatabase`**  
   - Przyjmuje parametry określające typ bazy, źródło danych, host, użytkownika, hasło i port.
   - Zwraca `bool` wskazujący sukces lub niepowodzenie połączenia.

2. **Zależności**  
   - Minimalne, tylko `QString` z Qt dla parametrów wejściowych.

## Opis działania
- **Cel funkcji**: Utworzenie i zarejestrowanie globalnego połączenia `QSqlDatabase` pod nazwą "default_connection", które jest współdzielone przez wszystkie komponenty aplikacji.
- **Obsługiwane bazy danych**:
  - **SQLite** (`QSQLITE`): Używa ścieżki do pliku jako `dbSource`. Automatycznie tworzy schemat bazy i przykładowe dane (implementacja w `utils.cpp`).
  - **MySQL** (`QMYSQL`): Używa nazwy bazy danych jako `dbSource`, wymaga parametrów `host`, `user`, `password`, `port`.
- **Parametry**:
  - `dbType`: Określa typ bazy ("SQLite3" lub "MySQL").
  - `dbSource`: Ścieżka do pliku (SQLite) lub nazwa bazy (MySQL).
  - `host`, `user`, `password`, `port`: Opcjonalne dla MySQL, ignorowane dla SQLite.
- **Zwracana wartość**: `true` po pomyślnym nawiązaniu połączenia, `false` w przypadku błędu (np. nieprawidłowe dane uwierzytelniające, brak pliku bazy).

## Zależności
- **Biblioteki Qt**:
  - `QString`: Do obsługi parametrów wejściowych.
  - Pośrednio: `QSqlDatabase` (używane w `utils.cpp`).
- **Nagłówki aplikacji**:
  - Brak zależności od innych nagłówków aplikacji.
- **Zasoby**:
  - Brak bezpośrednich zależności od zasobów Qt (np. QSS, czcionek).

## Uwagi
- Kod nie został zmodyfikowany, zgodnie z wymaganiami użytkownika.
- Funkcja `setupDatabase` jest centralnym punktem konfiguracji bazy danych, używanym przez inne komponenty aplikacji.
- Połączenie "default_connection" jest współdzielone, co wymaga ostrożnego zarządzania w przypadku wielokrotnego wywoływania funkcji.
- Obsługuje zarówno SQLite, jak i MySQL, co zwiększa elastyczność aplikacji.
- Dokumentacja implementacji (w `utils.cpp`) zawiera szczegóły dotyczące automatycznego tworzenia schematu dla SQLite.