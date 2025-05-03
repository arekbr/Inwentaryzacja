# Dokumentacja dla pliku `mainwindow.h`

## Metadane
- **Plik**: mainwindow.h
- **Wersja**: 1.2.2
- **Data**: 2025-05-03
- **Autorzy**: Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK & ChatGPT & GROK

## Przegląd
Plik `mainwindow.h` definiuje klasę `MainWindow`, która reprezentuje główne okno formularza do dodawania, edycji i klonowania eksponatów w aplikacji inwentaryzacyjnej. Klasa obsługuje ładowanie danych do combo boxów, zarządzanie zdjęciami (w bazie i buforze), tryby edycji/klonowania oraz komunikację z bazą danych MySQL. Emituje sygnał `recordSaved` po zapisaniu rekordu i zawiera sloty do obsługi akcji użytkownika, takich jak zapis, anulowanie czy dodawanie zdjęć.

## Struktura kodu
Plik nagłówkowy zawiera następujące elementy:

1. **Deklaracja klasy `MainWindow`**  
   - Dziedziczy po `QMainWindow`.
   - Używa makra `Q_OBJECT` dla wsparcia mechanizmu sygnałów i slotów Qt.

2. **Metody publiczne**  
   - Konstruktor: Inicjalizuje interfejs, połączenie z bazą danych i combo boxy.
   - Destruktor: Zwalnia zasoby.
   - `loadComboBoxData`: Ładuje dane z tabeli do combo boxa.
   - `eventFilter`: Obsługuje zdarzenia dla combo boxów (otwieranie listy po kliknięciu).
   - `setEditMode`: Ustawia tryb edycji lub dodawania rekordu.
   - `setCloneMode`: Ustawia tryb klonowania rekordu.
   - `getNewItemVendorComboBox`, `getNewItemModelComboBox`, `getNewItemStoragePlaceComboBox`, `getNewItemTypeComboBox`, `getNewItemStatusComboBox`: Zwracają wskaźniki na combo boxy.

3. **Sygnały**  
   - `recordSaved`: Emitowany po zapisaniu rekordu, przekazuje ID rekordu.

4. **Sloty prywatne**  
   - `onSaveClicked`: Zapisuje dane do bazy danych i emituje sygnał `recordSaved`.
   - `onCancelClicked`: Zamyka okno bez zapisywania zmian.
   - `onAddPhotoClicked`: Dodaje nowe zdjęcie do rekordu.
   - `onRemovePhotoClicked`: Usuwa wybrane zdjęcie.
   - `onPhotoClicked`: Obsługuje kliknięcie w miniaturę zdjęcia.
   - `onAddTypeClicked`, `onAddVendorClicked`, `onAddModelClicked`, `onAddStatusClicked`, `onAddStoragePlaceClicked`: Otwierają okna dialogowe dla słowników.

5. **Metody prywatne**  
   - `loadRecord`: Wczytuje dane rekordu do formularza.
   - `loadPhotos`: Wczytuje zdjęcia powiązane z rekordem.
   - `loadPhotosFromBuffer`: Wczytuje zdjęcia z bufora pamięci.

6. **Zmienne prywatne**  
   - `db`: Połączenie z bazą danych MySQL.
   - `ui`: Wskaźnik na interfejs użytkownika.
   - `m_editMode`: Flaga trybu edycji (true) lub dodawania (false).
   - `m_recordId`: ID edytowanego rekordu.
   - `m_selectedPhotoIndex`: Indeks wybranej miniatury zdjęcia.
   - `m_photoBuffer`: Bufor dla zdjęć niezapisanych rekordów.
   - `m_photoPathsBuffer`: Bufor ścieżek do zdjęć przed zapisem.

## Opis działania
- **Inicjalizacja**: Konstruktor konfiguruje interfejs, łączy się z bazą danych, ładuje dane do combo boxów, ustawia sloty dla przycisków i włącza automatyczne dodawanie wartości do słowników.
- **Formularz**: Wyświetla pola tekstowe (nazwa, numer seryjny, opis, itp.), combo boxy (typy, producenci, modele, statusy, miejsca przechowywania) i obszar dla miniatur zdjęć.
- **Tryby pracy**:
  - **Dodawanie**: Czyści pola, ustawia domyślne wartości ("brak" dla statusu i miejsca przechowywania).
  - **Edycja**: Wczytuje dane rekordu i zdjęcia na podstawie ID.
  - **Klonowanie**: Wczytuje dane rekordu, ale zapisuje jako nowy rekord (bez ID i zdjęć).
- **Zdjęcia**: Obsługuje dodawanie zdjęć (do bufora lub bazy), usuwanie (z bazy po potwierdzeniu) i wybór miniatur w `QGraphicsView`.
- **Słowniki**: Umożliwia dodawanie nowych wartości do tabel `types`, `vendors`, `models`, `statuses`, `storage_places` przez dialogi lub automatycznie po wpisaniu w combo boxie.
- **Zapis**: Waliduje dane, zapisuje rekord w tabeli `eksponaty`, przenosi zdjęcia do tabeli `photos` i katalogu "gotowe", emituje `recordSaved`.

## Zależności
- **Biblioteki Qt**:
  - `QMainWindow`, `QSqlDatabase`, `QComboBox`, `QList`: Obsługa interfejsu, bazy danych i danych.
- **Nagłówki aplikacji**:
  - `photoitem.h`: Klasa dla miniatur zdjęć (forward-deklaracja).
- **Interfejs użytkownika**:
  - `ui_mainwindow.h`: Plik generowany przez Qt Designer.
- **Zasoby**:
  - Brak bezpośrednich zależności od zasobów Qt (np. QSS, czcionek).

## Uwagi
- Kod nie został zmodyfikowany, zgodnie z wymaganiami użytkownika.
- Klasa integruje formularz z bazą danych i dialogami słownikowymi, będąc kluczowym elementem edycji danych.
- Automatyczne dodawanie wartości do słowników wymaga wybrania producenta dla modeli, co zapewnia spójność danych.
- Zdjęcia są przechowywane jako `LONGBLOB` w tabeli `photos`, co może wpłynąć na wydajność przy dużej liczbie rekordów.
- Połączenie z bazą danych (`default_connection`) jest współdzielone z innymi komponentami (np. `itemList`).