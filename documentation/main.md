# Dokumentacja dla pliku `main.cpp`

## Metadane
- **Plik**: main.cpp
- **Wersja**: 1.2.2
- **Data**: 2025-05-03
- **Autorzy**: Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK & ChatGPT & GROK

## Przegląd
Plik `main.cpp` jest głównym punktem wejścia aplikacji inwentaryzacyjnej opartej na bibliotece Qt. Odpowiada za inicjalizację aplikacji, ustawienie jej podstawowych właściwości (nazwa, wersja, ikona), ładowanie tłumaczeń językowych, wyświetlanie okna konfiguracji bazy danych oraz nawiązywanie połączenia z wybraną bazą danych (SQLite lub MySQL). Po poprawnym skonfigurowaniu środowiska uruchamia główne okno aplikacji reprezentowane przez klasę `itemList`.

## Struktura kodu
Kod jest zorganizowany w następujące sekcje funkcjonalne:

1. **Inicjalizacja aplikacji Qt**  
   - Tworzy instancję `QApplication` i ustawia styl graficzny "Fusion" dla spójnego wyglądu.
   - Ustawia nazwę aplikacji ("Inwentaryzacja") i wersję (1.2.2).
   - Ładuje i ustawia ikonę aplikacji z zasobów Qt (`:/images/icon.png`).

2. **Ładowanie tłumaczeń**  
   - Wczytuje tłumaczenia językowe na podstawie języka systemowego użytkownika.
   - Pliki tłumaczeń (`Inwentaryzacja_*.qm`) znajdują się w zasobach Qt (`:/i18n/`).
   - Po znalezieniu odpowiedniego pliku tłumaczenia, instaluje go w aplikacji.

3. **Konfiguracja bazy danych**  
   - Wyświetla okno dialogowe `DatabaseConfigDialog`, w którym użytkownik wybiera typ bazy danych (SQLite lub MySQL) i wprowadza parametry połączenia.
   - Jeśli użytkownik anuluje konfigurację, aplikacja kończy działanie.

4. **Nawiązywanie połączenia z bazą danych**  
   - Pobiera wybrany typ bazy danych i odpowiednie parametry z `DatabaseConfigDialog`.
   - Obsługuje dwa typy baz danych:
     - **SQLite**: Sprawdza ścieżkę do pliku bazy i wywołuje funkcję `setupDatabase` z parametrem ścieżki.
     - **MySQL**: Przekazuje dane dostępowe (host, nazwa bazy, użytkownik, hasło, port) do funkcji `setupDatabase`.
   - Loguje błędy w przypadku niepoprawnego typu bazy danych lub nieudanego połączenia.

5. **Uruchamianie głównego okna**  
   - Tworzy i wyświetla główne okno aplikacji (`itemList`), które zawiera interfejs użytkownika do zarządzania inwentaryzacją.

6. **Pętla zdarzeń Qt**  
   - Uruchamia główną pętlę zdarzeń Qt za pomocą `QApplication::exec()`, obsługującą interakcje użytkownika i zdarzenia systemowe.

## Opis działania
1. **Inicjalizacja**: Po uruchomieniu aplikacji tworzona jest instancja `QApplication`, która zarządza cyklem życia aplikacji Qt. Styl "Fusion" zapewnia spójny wygląd na różnych platformach.
2. **Tłumaczenia**: Aplikacja próbuje załadować tłumaczenia na podstawie języka systemowego, co umożliwia lokalizację interfejsu użytkownika.
3. **Konfiguracja bazy danych**: Użytkownik musi skonfigurować połączenie z bazą danych za pomocą okna `DatabaseConfigDialog`. Jeśli konfiguracja zostanie anulowana, aplikacja kończy działanie.
4. **Połączenie z bazą danych**: Funkcja `setupDatabase` (zdefiniowana w `utils.h`) jest wywoływana z odpowiednimi parametrami w zależności od typu bazy danych. W przypadku niepowodzenia aplikacja kończy działanie.
5. **Główne okno**: Po pomyślnym połączeniu z bazą danych wyświetlane jest główne okno aplikacji, które umożliwia użytkownikowi interakcję z danymi inwentaryzacyjnymi.
6. **Pętla zdarzeń**: Aplikacja przechodzi w tryb oczekiwania na zdarzenia użytkownika, takie jak kliknięcia czy wprowadzanie danych.

## Zależności
- **Biblioteki Qt**:
  - `QApplication`: Zarządzanie aplikacją i cyklem zdarzeń.
  - `QCoreApplication`: Ustawianie metadanych aplikacji.
  - `QDebug`: Logowanie komunikatów diagnostycznych.
  - `QDirIterator`: (Nieużywane wprost, ale włączone w pliku).
  - `QTranslator`: Obsługa tłumaczeń językowych.
  - `QIcon`: Ustawianie ikony aplikacji.
- **Nagłówki aplikacji**:
  - `DatabaseConfigDialog.h`: Okno konfiguracji bazy danych.
  - `itemList.h`: Główne okno aplikacji.
  - `utils.h`: Funkcje pomocnicze, w tym `setupDatabase`.
- **Zasoby**:
  - Ikona aplikacji (`:/images/icon.png`).
  - Pliki tłumaczeń (`:/i18n/Inwentaryzacja_*.qm`).

## Uwagi
- Kod nie został zmodyfikowany, zgodnie z wymaganiami użytkownika.
- Aplikacja wymaga poprawnego skonfigurowania bazy danych, aby przejść do głównego okna.
- Styl graficzny i czcionki są zarządzane przez `DatabaseConfigDialog` po zatwierdzeniu ustawień.
- W przypadku błędów (np. brak pliku SQLite lub niepoprawny typ bazy danych) aplikacja loguje komunikaty za pomocą `qDebug` i kończy działanie.

## Wartości zwracane
- **0**: Aplikacja zakończyła się pomyślnie lub użytkownik anulował konfigurację.
- **Inne**: Błąd podczas nawiązywania połączenia z bazą danych lub nieznany typ bazy danych.