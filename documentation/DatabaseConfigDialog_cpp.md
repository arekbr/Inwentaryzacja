# Dokumentacja dla pliku `DatabaseConfigDialog.cpp`

## Metadane
- **Plik**: DatabaseConfigDialog.cpp
- **Wersja**: 1.2.2
- **Data**: 2025-05-03
- **Autorzy**: Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK & ChatGPT & GROK

## Przegląd
Plik `DatabaseConfigDialog.cpp` zawiera implementację metod klasy `DatabaseConfigDialog`, odpowiedzialnej za konfigurację parametrów połączenia z bazą danych (SQLite lub MySQL) oraz wybór skórki graficznej aplikacji (Amiga, Atari 8bit, ZX Spectrum, Standard). Implementacja obejmuje ładowanie stylów QSS, czcionek, zapis ustawień w `QSettings` oraz obsługę dynamicznego wyboru pliku SQLite z możliwością tworzenia nowego pliku `.db`.

## Struktura kodu
Kod jest podzielony na następujące sekcje:

1. **Sekcja pomocnicza**  
   - Funkcja `getSettings()`: Tworzy i zwraca obiekt `QSettings` dla pliku `inwentaryzacja.ini`.

2. **Sekcja konstruktora**  
   - Inicjalizuje interfejs użytkownika, combo boxy, wczytuje zapisane ustawienia i ustanawia połączenia sygnałów-slotów.

3. **Sekcja destruktora**  
   - Zwalnia zasoby interfejsu i odrejestrowuje czcionki.

4. **Sekcja metod publicznych**  
   - Metody dostępowe do parametrów bazy danych.
   - Metoda `accept()`: Zapisuje ustawienia i zamyka okno dialogowe.

5. **Sekcja slotów prywatnych**  
   - `onDatabaseTypeChanged(int index)`: Przełącza interfejs w zależności od typu bazy danych.
   - `onSkinChanged(const QString &skin)`: Aktualizuje styl i czcionkę dla wybranej skórki.

6. **Sekcja metod prywatnych**  
   - `loadStyleSheet(const QString &skin)`: Ładuje plik QSS dla wybranej skórki.
   - `loadFont(const QString &skin)`: Ładuje czcionkę dla wybranej skórki.

## Opis działania
1. **Inicjalizacja**:
   - Konstruktor konfiguruje interfejs użytkownika (`ui->setupUi`), wypełnia combo boxy opcjami bazy danych (SQLite3, MySQL) i skórek graficznych (Amiga, Atari 8bit, ZX Spectrum, Standard).
   - Wczytuje zapisane ustawienia z `QSettings` i ustawia początkowe wartości pól (np. ścieżka SQLite, dane MySQL, wybrana skórka).
   - Ustanawia połączenia sygnałów-slotów dla przycisków, combo boxów i wyboru pliku SQLite.

2. **Wybór pliku SQLite**:
   - Przycisk `selectFileButton` otwiera okno dialogowe `QFileDialog::getSaveFileName`, umożliwiając wybór istniejącego lub nowego pliku `.db`.
   - Jeśli wybrany plik nie istnieje, tworzony jest pusty plik.

3. **Zmiana typu bazy danych**:
   - Slot `onDatabaseTypeChanged` przełącza strony w `QStackedWidget`, pokazując pola odpowiednie dla SQLite lub MySQL.

4. **Zmiana skórki graficznej**:
   - Slot `onSkinChanged` wywołuje `loadStyleSheet` i `loadFont`, aktualizując styl i czcionkę aplikacji.
   - Wybór skórki jest zapisywany w `QSettings`.

5. **Zatwierdzanie ustawień**:
   - Metoda `accept` zapisuje wszystkie parametry (baza danych, skórka) w `QSettings` i zamyka okno dialogowe.

6. **Ładowanie stylów i czcionek**:
   - `loadStyleSheet` wczytuje plik QSS z zasobów Qt i stosuje go za pomocą `qApp->setStyleSheet`.
   - `loadFont` ładuje czcionkę (np. `topaz.ttf` dla Amigi) za pomocą `QFontDatabase`, ustawia ją dla aplikacji i resetuje paletę/styl dla propagacji zmian.

## Zależności
- **Biblioteki Qt**:
  - `QApplication`: Ustawianie stylów i czcionek dla aplikacji.
  - `QDebug`, `qWarning`: Logowanie komunikatów diagnostycznych.
  - `QDir`, `QFile`: Obsługa plików konfiguracyjnych i zasobów.
  - `QFileDialog`: Wybór pliku SQLite.
  - `QFontDatabase`: Ładowanie i zarządzanie czcionkami.
  - `QPushButton`, `QSettings`: Obsługa interfejsu i zapisu ustawień.
- **Interfejs użytkownika**:
  - `ui_DatabaseConfigDialog.h`: Plik generowany przez Qt Designer.
- **Zasoby**:
  - Pliki QSS (`:/styles/amiga.qss`, `:/styles/atari8bit.qss`, `:/styles/zxspectrum.qss`, `:/styles/default.qss`).
  - Czcionki (`:/fonts/topaz.ttf`, `:/fonts/zxspectrum.ttf`, `:/fonts/EightBit Atari-Ataripl.ttf`).

## Uwagi
- Kod nie został zmodyfikowany, zgodnie z wymaganiami użytkownika.
- Ustawienia są zapisywane w pliku `inwentaryzacja.ini` w katalogu aplikacji.
- W przypadku błędów ładowania zasobów (np. czcionek lub QSS) aplikacja przechodzi na domyślne ustawienia i loguje ostrzeżenia.
- Czcionki są odrejestrowywane w destruktorze, aby zapobiec wyciekom pamięci.