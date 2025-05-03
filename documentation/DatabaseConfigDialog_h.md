# Dokumentacja dla pliku `DatabaseConfigDialog.h`

## Metadane
- **Plik**: DatabaseConfigDialog.h
- **Wersja**: 1.2.2
- **Data**: 2025-05-03
- **Autorzy**: Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK & ChatGPT & GROK

## Przegląd
Plik `DatabaseConfigDialog.h` definiuje klasę `DatabaseConfigDialog`, która reprezentuje okno dialogowe do konfiguracji parametrów połączenia z bazą danych (SQLite lub MySQL) oraz wyboru skórki graficznej aplikacji (Amiga, Atari 8bit, ZX Spectrum, Standard). Klasa dostarcza metody dostępowe do parametrów bazy danych, obsługuje dynamiczną zmianę stylów i czcionek oraz zapisuje ustawienia w `QSettings` dla trwałości między sesjami aplikacji.

## Struktura kodu
Plik nagłówkowy zawiera następujące elementy:

1. **Deklaracja klasy `DatabaseConfigDialog`**  
   - Klasa dziedziczy po `QDialog` i używa mechanizmu sygnałów i slotów Qt (`Q_OBJECT`).
   - Zawiera publiczne i prywatne metody oraz zmienne.

2. **Metody publiczne**  
   - Konstruktor: Inicjalizuje okno dialogowe i interfejs użytkownika.
   - Destruktor: Zwalnia zasoby i odrejestrowuje czcionki.
   - Metody dostępowe: Zwracają parametry bazy danych (typ, ścieżka SQLite, dane MySQL).

3. **Sloty publiczne**  
   - `accept()`: Przesłania metodę `QDialog::accept` do zapisu ustawień w `QSettings`.

4. **Sloty prywatne**  
   - `onDatabaseTypeChanged(int index)`: Obsługuje zmianę typu bazy danych w combo boxie.
   - `onSkinChanged(const QString &skin)`: Obsługuje zmianę skórki graficznej.

5. **Metody prywatne**  
   - `loadStyleSheet(const QString &skin)`: Ładuje arkusz stylów QSS dla wybranej skórki.
   - `loadFont(const QString &skin)`: Ładuje czcionkę dla wybranej skórki.

6. **Zmienne prywatne**  
   - `ui`: Wskaźnik na obiekt interfejsu użytkownika.
   - `m_topazFontId`, `m_zxFontId`, `m_atari8bitFontId`: Identyfikatory załadowanych czcionek.

## Opis działania
Klasa `DatabaseConfigDialog` odpowiada za:
- **Wyświetlanie okna dialogowego**: Użytkownik wybiera typ bazy danych (SQLite/MySQL) i wprowadza parametry połączenia za pomocą interfejsu graficznego.
- **Dynamiczne przełączanie interfejsu**: Używa `QStackedWidget` do pokazywania odpowiednich pól w zależności od typu bazy danych.
- **Wybór skórki graficznej**: Umożliwia wybór jednej z czterech skórek (Amiga, Atari 8bit, ZX Spectrum, Standard), co wpływa na styl (QSS) i czcionkę aplikacji.
- **Trwałe przechowywanie ustawień**: Parametry bazy danych i wybrana skórka są zapisywane w pliku `inwentaryzacja.ini` za pomocą `QSettings`.
- **Metody dostępowe**: Umożliwiają innym częściom aplikacji (np. `main.cpp`) pobranie skonfigurowanych parametrów.

## Zależności
- **Biblioteki Qt**:
  - `QDialog`: Bazowa klasa dla okna dialogowego.
  - `QString`: Obsługa ciągów znaków dla parametrów.
- **Namespace `Ui`**:
  - Zawiera definicję interfejsu użytkownika (`ui_DatabaseConfigDialog.h`) generowaną przez Qt Designer.
- **Zasoby**:
  - Pliki QSS dla stylów (`:/styles/amiga.qss`, `:/styles/atari8bit.qss`, `:/styles/zxspectrum.qss`, `:/styles/default.qss`).
  - Czcionki (`:/fonts/topaz.ttf`, `:/fonts/zxspectrum.ttf`, `:/fonts/EightBit Atari-Ataripl.ttf`).

## Uwagi
- Kod nie został zmodyfikowany, zgodnie z wymaganiami użytkownika.
- Klasa dynamicznie dostosowuje interfejs użytkownika do wybranego typu bazy danych.
- Czcionki są ładowane z zasobów Qt i odrejestrowywane w destruktorze, aby zapobiec wyciekom pamięci.
- W przypadku błędów ładowania zasobów (np. czcionek lub QSS) aplikacja przechodzi na domyślne ustawienia (czcionka systemowa, brak stylu).