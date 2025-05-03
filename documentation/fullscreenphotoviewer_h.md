# Dokumentacja dla pliku `fullscreenphotoviewer.h`

## Metadane
- **Plik**: fullscreenphotoviewer.h
- **Wersja**: 1.2.2
- **Data**: 2025-05-03
- **Autorzy**: Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK & ChatGPT & GROK

## Przegląd
Plik `fullscreenphotoviewer.h` definiuje dwie klasy: `ZoomableGraphicsView` oraz `FullScreenPhotoViewer`, używane w aplikacji inwentaryzacyjnej do pełnoekranowego podglądu zdjęć eksponatów. `ZoomableGraphicsView` umożliwia powiększanie i przesuwanie obrazu za pomocą kółka myszy i przeciągania, natomiast `FullScreenPhotoViewer` tworzy bezramkowe, modalne okno pełnoekranowe z możliwością zamknięcia klawiszem Escape.

## Struktura kodu
Plik nagłówkowy zawiera następujące elementy:

1. **Deklaracja klasy `ZoomableGraphicsView`**  
   - Dziedziczy po `QGraphicsView`.
   - Metody:
     - Konstruktor: Inicjalizuje widok z trybem przeciągania i kotwicą transformacji.
     - `wheelEvent`: Obsługuje powiększanie/pomniejszanie obrazu kółkiem myszy.
   - Zmienna: `m_scaleFactor` (bieżący współczynnik skali).

2. **Deklaracja klasy `FullScreenPhotoViewer`**  
   - Dziedziczy po `QMainWindow`.
   - Metody:
     - Konstruktor: Inicjalizuje okno pełnoekranowe, ustawia zdjęcie i widok.
     - `keyPressEvent`: Obsługuje zamykanie okna klawiszem Escape.
   - Zmienne: `m_scene` (scena graficzna), `m_view` (widok ZoomableGraphicsView).

3. **Metody inline**  
   - Wszystkie metody są zdefiniowane w pliku nagłówkowym, co eliminuje potrzebę osobnego pliku .cpp.

## Opis działania
1. **ZoomableGraphicsView**:
   - **Inicjalizacja**: Ustawia kotwicę transformacji pod kursorem myszy (`AnchorUnderMouse`) i tryb przeciągania (`ScrollHandDrag`), umożliwiając intuicyjne powiększanie i przesuwanie obrazu.
   - **Powiększanie**: Kółko myszy zwiększa skalę o 1.15 (zoom in) lub zmniejsza o 1/1.15 (zoom out). Współczynnik skali jest aktualizowany w zmiennej `m_scaleFactor`.
   - **Przesuwanie**: Tryb ScrollHandDrag pozwala na przesuwanie obrazu przez przeciąganie myszą.

2. **FullScreenPhotoViewer**:
   - **Inicjalizacja**: Tworzy bezramkowe okno pełnoekranowe z flagami `FramelessWindowHint` i `WindowStaysOnTopHint`. Ustawia modalność (`ApplicationModal`) i automatyczne usuwanie po zamknięciu (`WA_DeleteOnClose`).
   - **Wyświetlanie zdjęcia**: Zdjęcie (`QPixmap`) jest dodawane do sceny graficznej (`QGraphicsScene`), która jest wyświetlana w `ZoomableGraphicsView` ustawionym jako centralny widget.
   - **Zamykanie**: Okno zamyka się po naciśnięciu klawisza Escape, co jest obsługiwane w metodzie `keyPressEvent`.

## Zależności
- **Biblioteki Qt**:
  - `QMainWindow`: Bazowa klasa dla okna pełnoekranowego.
  - `QGraphicsView`, `QGraphicsScene`: Obsługa widoku i sceny graficznej.
  - `QKeyEvent`, `QWheelEvent`: Obsługa zdarzeń klawiatury i kółka myszy.
- **Zasoby**:
  - Brak bezpośrednich zależności od zasobów Qt (np. QSS, czcionek).

## Uwagi
- Kod nie został zmodyfikowany, zgodnie z wymaganiami użytkownika.
- Implementacja inline w pliku nagłówkowym zapewnia prostotę i wydajność.
- `FullScreenPhotoViewer` jest modalny, co blokuje interakcję z innymi oknami aplikacji podczas wyświetlania zdjęcia.
- Automatyczne usuwanie po zamknięciu (`WA_DeleteOnClose`) zapobiega wyciekom pamięci.