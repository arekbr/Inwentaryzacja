# Dokumentacja dla pliku `mainwindow.cpp`

## Metadane
- **Plik**: mainwindow.cpp
- **Wersja**: 1.2.2
- **Data**: 2025-05-03
- **Autorzy**: Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK & ChatGPT & GROK

## Przegląd
Plik `mainwindow.cpp` zawiera implementację metod klasy `MainWindow`, odpowiedzialnej za formularz do dodawania, edycji i klonowania eksponatów w aplikacji inwentaryzacyjnej. Klasa obsługuje ładowanie danych do combo boxów, zarządzanie zdjęciami (w bazie i buforze), zapisywanie rekordów w bazie danych MySQL oraz otwieranie dialogów słownikowych. Implementacja obejmuje również automatyczne dodawanie nowych wartości do słowników po ich wpisaniu w combo boxach.

## Struktura kodu
Kod jest podzielony na następujące sekcje:

1. **Sekcja konstruktora**  
   - Inicjalizuje interfejs, połączenie z bazą danych, combo boxy, sloty i automatyczne dodawanie wartości do słowników.

2. **Sekcja destruktora**  
   - Zamyka połączenie z bazą danych i zwalnia zasoby.

3. **Sekcja metod publicznych**  
   - `loadComboBoxData`: Ładuje dane do combo boxów.
   - `eventFilter`: Obsługuje kliknięcia w combo boxy.
   - `setEditMode`, `setCloneMode`: Konfigurują tryby formularza.
   - `getNewItem*ComboBox`: Zwracają wskaźniki na combo boxy.

4. **Sekcja slotów prywatnych**  
   - `onSaveClicked`: Zapisuje rekord i zdjęcia.
   - `onCancelClicked`: Zamyka okno.
   - `onAddPhotoClicked`, `onRemovePhotoClicked`, `onPhotoClicked`: Zarządzają zdjęciami.
   - `onAdd*Clicked`: Otwierają dialogi słownikowe.

5. **Sekcja metod prywatnych**  
   - `loadRecord`: Wczytuje dane rekordu.
   - `loadPhotos`, `loadPhotosFromBuffer`: Wczytują zdjęcia z bazy lub bufora.

## Opis działania
1. **Inicjalizacja**:
   - Konstruktor ustawia interfejs (`ui->setupUi`), pobiera istniejące połączenie z bazą MySQL (`default_connection`), ładuje dane do combo boxów (`types`, `vendors`, `models`, `statuses`, `storage_places`), podłącza sloty dla przycisków i konfiguruje automatyczne dodawanie wartości do słowników poprzez sygnał `editingFinished` dla pól edycyjnych combo boxów.
   - Ustawia początkowy stan: brak trybu edycji, brak wybranego zdjęcia.

2. **Formularz**:
   - Zawiera pola tekstowe (nazwa, numer seryjny, part number, rewizja, opis, wartość), pole daty (rok produkcji), combo boxy (typy, producenci, modele, statusy, miejsca przechowywania) i obszar `QGraphicsView` dla miniatur zdjęć.
   - Combo boxy są edytowalne z autouzupełnianiem (`QCompleter`).

3. **Tryby pracy**:
   - **Dodawanie**: Czyści pola, ustawia domyślne wartości ("brak" dla statusu i miejsca przechowywania), przechowuje zdjęcia w buforze.
   - **Edycja**: Wczytuje dane rekordu i zdjęcia na podstawie ID, zapisuje zmiany w istniejącym rekordzie.
   - **Klonowanie**: Wczytuje dane rekordu, ale zapisuje jako nowy rekord (bez ID i zdjęć).

4. **Zdjęcia**:
   - `onAddPhotoClicked`: Otwiera okno wyboru plików, ładuje zdjęcia do bufora (`m_photoBuffer`) dla nowych rekordów lub do bazy (`photos`) dla edycji, przenosi pliki do katalogu "gotowe".
   - `onRemovePhotoClicked`: Usuwa zdjęcie z bazy po potwierdzeniu, odświeżając podgląd.
   - `onPhotoClicked`: Ustawia wybraną miniaturę jako aktywną, aktualizując `m_selectedPhotoIndex`.
   - Miniatury (80x80 pikseli) są wyświetlane w `QGraphicsView` z zachowaniem proporcji.

5. **Słowniki**:
   - Automatyczne dodawanie: Po wpisaniu nowej wartości w combo boxie, zapisuje ją do odpowiedniej tabeli (`types`, `vendors`, `models`, `statuses`, `storage_places`) z nowym UUID. Dla modeli wymaga wybranego producenta.
   - Dialogi słownikowe (`onAdd*Clicked`): Otwierają okna (`types`, `vendors`, `models`, `status`, `storage`), przekazują referencję do `MainWindow` i odświeżają combo boxy po dodaniu wartości.

6. **Zapis**:
   - `onSaveClicked`: Waliduje dane, wykonuje `INSERT` (nowy rekord) lub `UPDATE` (edycja) w tabeli `eksponaty`, zapisuje zdjęcia z bufora do tabeli `photos`, przenosi pliki do katalogu "gotowe", emituje `recordSaved` i zamyka okno.
   - Używa domyślnych UUID dla pustych wartości combo boxów (np. `unknown_status_uuid`).

## Zależności
- **Biblioteki Qt**:
  - `QSqlQuery`, `QFileDialog`, `QMessageBox`, `QGraphicsScene`, `QCompleter`, `QDir`, `QUuid`, itp.: Obsługa bazy danych, plików, interfejsu i grafik.
- **Nagłówki aplikacji**:
  - `mainwindow.h`: Definicja klasy.
  - `photoitem.h`: Klasa dla miniatur zdjęć.
  - `types.h`, `vendors.h`, `models.h`, `status.h`, `storage.h`: Dialogi słownikowe.
- **Interfejs użytkownika**:
  - `ui_mainwindow.h`: Plik generowany przez Qt Designer.
- **Zasoby**:
  - Brak bezpośrednich zależności od zasobów Qt (np. QSS, czcionek).

## Uwagi
- Kod nie został zmodyfikowany, zgodnie z wymaganiami użytkownika.
- Implementacja zakłada użycie MySQL, ale aplikacja wspiera także SQLite (konfigurowane w `DatabaseConfigDialog`).
- Automatyczne dodawanie wartości do słowników zwiększa wygodę, ale wymaga walidacji (np. producent dla modeli).
- Zdjęcia przechowywane jako `LONGBLOB` mogą obciążać bazę przy dużej liczbie rekordów.
- Katalog "gotowe" jest tworzony w ścieżce źródłowej zdjęć, co wymaga uprawnień do zapisu.
- Połączenie z bazą danych (`default_connection`) jest współdzielone z innymi komponentami (np. `itemList`).