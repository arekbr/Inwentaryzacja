# Dokumentacja dla pliku `photoitem.h`

## Metadane
- **Plik**: photoitem.h
- **Wersja**: 1.2.2
- **Data**: 2025-05-03
- **Autorzy**: Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK & ChatGPT & GROK

## Przegląd
Plik `photoitem.h` definiuje klasę `PhotoItem`, która reprezentuje interaktywny element graficzny (miniaturę zdjęcia) w scenie graficznej Qt (używany w `QGraphicsView`). Klasa rozszerza `QGraphicsPixmapItem` o obsługę zdarzeń myszy (kliknięcia, podwójne kliknięcia) i hover (najechanie, opuszczenie kursora), a także wizualne zaznaczanie za pomocą czerwonej ramki. Dzięki dziedziczeniu po `QObject`, umożliwia emisję sygnałów (`clicked`, `doubleClicked`, `hovered`, `unhovered`), co pozwala na integrację z logiką aplikacji, np. z klasą `MainWindow` w aplikacji inwentaryzacyjnej.

## Struktura kodu
Plik nagłówkowy zawiera następujące elementy:

1. **Deklaracja klasy `PhotoItem`**  
   - Dziedziczy po `QObject` i `QGraphicsPixmapItem`.
   - Używa makra `Q_OBJECT` dla wsparcia mechanizmu sygnałów i slotów Qt.

2. **Metody publiczne**  
   - Konstruktor: Inicjalizuje obiekt, ramkę graficzną i włącza zdarzenia hover.
   - Destruktor: Zwalnia zasoby ramki.
   - `setSelected`: Ustawia stan zaznaczenia (wyświetla/ukrywa ramkę).

3. **Sygnały**  
   - `clicked`: Emitowany po kliknięciu (np. prawym przyciskiem) dla wyboru/podglądu zdjęcia.
   - `doubleClicked`: Emitowany po podwójnym kliknięciu (lewym przyciskiem) dla pełnoekranowego podglądu.
   - `hovered`: Emitowany po najechaniu kursorem na miniaturę.
   - `unhovered`: Emitowany po opuszczeniu miniatury kursorem.

4. **Metody chronione**  
   - `mousePressEvent`: Obsługuje kliknięcia myszą.
   - `mouseDoubleClickEvent`: Obsługuje podwójne kliknięcia.
   - `hoverEnterEvent`: Obsługuje najechanie kursorem.
   - `hoverLeaveEvent`: Obsługuje opuszczenie kursora.

5. **Metody prywatne**  
   - `updateFrame`: Aktualizuje wygląd ramki zaznaczenia.

6. **Zmienne prywatne**  
   - `m_pressed`: Flaga kliknięcia (obecnie nieużywana).
   - `m_selected`: Flaga zaznaczenia (kontroluje ramkę).
   - `m_frame`: Obiekt `QGraphicsRectItem` dla ramki zaznaczenia.

## Opis działania
- **Inicjalizacja**: Konstruktor tworzy obiekt jako element graficzny (`QGraphicsPixmapItem`) i obiekt sygnałowy (`QObject`), ustawia flagi `m_pressed` i `m_selected` na `false`, tworzy ramkę (`m_frame`) jako dziecko, włącza zdarzenia hover i konfiguruje ramkę jako niewidoczną.
- **Interfejs graficzny**: Wyświetla miniaturę zdjęcia (np. 80x80 pikseli w `MainWindow`) w `QGraphicsView`, z możliwością zaznaczenia czerwoną ramką o szerokości 2 piksele.
- **Interakcje**:
  - **Kliknięcia**: Prawe kliknięcie emituje `clicked` (np. dla podglądu), lewe jest przekazywane dla podwójnego kliknięcia.
  - **Podwójne kliknięcia**: Lewe podwójne kliknięcie emituje `doubleClicked` (np. dla pełnego ekranu).
  - **Hover**: Najechanie emituje `hovered`, opuszczenie `unhovered` (np. dla podświetlenia).
- **Zaznaczenie**: Metoda `setSelected` włącza/wyłącza czerwoną ramkę poprzez `updateFrame`, co jest używane w `MainWindow` do wskazywania wybranego zdjęcia.
- **Integracja**: Sygnały są podłączane w `MainWindow` do slotów, np. `onPhotoClicked`, dla wyboru lub podglądu zdjęcia.

## Zależności
- **Biblioteki Qt**:
  - `QGraphicsPixmapItem`, `QObject`, `QGraphicsRectItem`, `QGraphicsSceneHoverEvent`, `QGraphicsSceneMouseEvent`: Obsługa grafiki, sygnałów i zdarzeń.
- **Nagłówki aplikacji**:
  - Brak zależności od innych nagłówków aplikacji (np. `mainwindow.h`).
- **Zasoby**:
  - Brak bezpośrednich zależności od zasobów Qt (np. QSS, czcionek).

## Uwagi
- Kod nie został zmodyfikowany, zgodnie z wymaganiami użytkownika.
- Klasa jest kluczowym elementem dla interaktywnych miniatur zdjęć w `MainWindow`.
- Flaga `m_pressed` jest zdefiniowana, ale nieużywana, co sugeruje możliwość przyszłych rozszerzeń (np. obsługa przeciągania).
- Sygnały `hovered` i `unhovered` nie są obecnie wykorzystywane w `MainWindow`, ale mogą służyć do podświetlania lub tooltipów.
- Ramka zaznaczenia jest prosta (czerwona, 2 piksele), ale może być dostosowana (np. kolor, styl) w przyszłych wersjach.