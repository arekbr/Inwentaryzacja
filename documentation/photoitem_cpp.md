# Dokumentacja dla pliku `photoitem.cpp`

## Metadane
- **Plik**: photoitem.cpp
- **Wersja**: 1.2.2
- **Data**: 2025-05-03
- **Autorzy**: Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK & ChatGPT & GROK

## Przegląd
Plik `photoitem.cpp` zawiera implementację metod klasy `PhotoItem`, która reprezentuje interaktywną miniaturę zdjęcia w scenie graficznej Qt (`QGraphicsView`) w aplikacji inwentaryzacyjnej. Klasa rozszerza `QGraphicsPixmapItem` o obsługę zdarzeń myszy (kliknięcia, podwójne kliknięcia) i hover (najechanie, opuszczenie), a także wizualne zaznaczanie za pomocą czerwonej ramki. Dzięki dziedziczeniu po `QObject`, umożliwia emisję sygnałów (`clicked`, `doubleClicked`, `hovered`, `unhovered`) do integracji z logiką aplikacji, np. z klasą `MainWindow` dla wyboru lub podglądu zdjęć eksponatów.

## Struktura kodu
Kod jest podzielony na następujące sekcje:

1. **Sekcja konstruktora**  
   - Inicjalizuje obiekt, ramkę graficzną i włącza zdarzenia hover.

2. **Sekcja destruktora**  
   - Zwalnia zasoby ramki.

3. **Sekcja metod publicznych**  
   - `setSelected`: Ustawia stan zaznaczenia i aktualizuje ramkę.

4. **Sekcja metod chronionych**  
   - `mousePressEvent`: Obsługuje kliknięcia myszą.
   - `mouseDoubleClickEvent`: Obsługuje podwójne kliknięcia.
   - `hoverEnterEvent`: Obsługuje najechanie kursorem.
   - `hoverLeaveEvent`: Obsługuje opuszczenie kursora.

5. **Sekcja metod prywatnych**  
   - `updateFrame`: Aktualizuje wygląd ramki zaznaczenia.

## Opis działania
1. **Inicjalizacja**:
   - Konstruktor tworzy obiekt jako `QObject` i `QGraphicsPixmapItem`, ustawia flagi `m_pressed` i `m_selected` na `false`, tworzy ramkę (`m_frame`) jako dziecko, włącza zdarzenia hover (`setAcceptHoverEvents(true)`), wyłącza zdarzenia dotykowe i ustawia ramkę jako niewidoczną (`Qt::NoPen`, `Qt::NoBrush`).
   - Ramka ma wyższy priorytet rysowania (`setZValue(1)`), aby była widoczna nad miniaturą.

2. **Interfejs graficzny**:
   - Wyświetla miniaturę zdjęcia (np. 80x80 pikseli w `MainWindow`) w `QGraphicsView`.
   - Przy zaznaczeniu (`m_selected == true`), pokazuje czerwoną ramkę o szerokości 2 piksele, obejmującą miniaturę z marginesem (-2, +2 piksele).

3. **Interakcje**:
   - **Kliknięcia** (`mousePressEvent`):
     - Prawe kliknięcie emituje `clicked` (np. dla podglądu zdjęcia w `MainWindow`).
     - Lewe kliknięcie jest przekazywane do `QGraphicsPixmapItem`, aby umożliwić wykrycie podwójnego kliknięcia.
     - Inne przyciski są obsługiwane domyślnie.
   - **Podwójne kliknięcia** (`mouseDoubleClickEvent`):
     - Lewe podwójne kliknięcie emituje `doubleClicked` (np. dla pełnoekranowego podglądu).
     - Inne przyciski są obsługiwane domyślnie.
   - **Hover**:
     - Najechanie (`hoverEnterEvent`) emituje `hovered` (np. dla podświetlenia, nieużywane w `MainWindow`).
     - Opuszczenie (`hoverLeaveEvent`) emituje `unhovered` (np. dla usunięcia podświetlenia).
   - Wszystkie zdarzenia są przekazywane do `QGraphicsPixmapItem` dla domyślnej obsługi.

4. **Zaznaczenie**:
   - `setSelected` ustawia `m_selected` i wywołuje `updateFrame`.
   - `updateFrame` rysuje czerwoną ramkę (2 piksele) dla `m_selected == true` lub ukrywa ją (`Qt::NoPen`) dla `m_selected == false`.
   - Ramka jest dopasowana do granic miniatury (`boundingRect`) z marginesem.

5. **Integracja**:
   - Sygnał `clicked` jest podłączony w `MainWindow` do `onPhotoClicked` dla wyboru miniatury (np. do usunięcia lub podglądu).
   - Sygnał `doubleClicked` może być użyty do pełnoekranowego podglądu (niezrealizowane w `MainWindow`).
   - Sygnały `hovered` i `unhovered` są zdefiniowane, ale nieużywane w obecnej implementacji.

## Zależności
- **Biblioteki Qt**:
  - `QGraphicsSceneMouseEvent`, `QPainter`, `QPen`, `QUuid`: Obsługa zdarzeń myszy, rysowania i generowania UUID.
- **Nagłówki aplikacji**:
  - `photoitem.h`: Definicja klasy.
- **Zasoby**:
  - Brak bezpośrednich zależności od zasobów Qt (np. QSS, czcionek).

## Uwagi
- Kod nie został zmodyfikowany, zgodnie z wymaganiami użytkownika.
- Klasa jest zoptymalizowana dla miniatur zdjęć w `MainWindow`, ale sygnały `hovered` i `unhovered` nie są wykorzystywane, co sugeruje możliwość dodania podświetlania lub tooltipów.
- Flaga `m_pressed` jest zdefiniowana, ale nieużywana, co może wskazywać na planowane funkcje (np. przeciąganie miniatur).
- Ramka zaznaczenia jest statyczna (czerwona, 2 piksele), ale może być dostosowana (np. kolor, grubość) w przyszłych wersjach.
- Obsługa prawego przycisku dla `clicked` może być rozszerzona o menu kontekstowe (np. "Usuń", "Podgląd").
- Klasa nie zależy od bazy danych ani innych komponentów aplikacji, co czyni ją uniwersalną dla scen graficznych Qt.