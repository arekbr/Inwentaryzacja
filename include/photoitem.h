/**
 * @file photoitem.h
 * @brief Deklaracja klasy PhotoItem reprezentującej interaktywny element graficzny zdjęcia.
 * @author Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK & ChatGPT & GROK
 * @version 1.2.2
 * @date 2025-05-03
 *
 * @section Overview
 * Plik zawiera deklarację klasy PhotoItem, która rozszerza funkcjonalność QGraphicsPixmapItem
 * o obsługę interakcji użytkownika (kliknięcia, najechanie kursorem, opuszczenie kursora)
 * oraz możliwość wizualnego zaznaczenia elementu za pomocą ramki. Klasa jest używana
 * w aplikacji inwentaryzacyjnej do wyświetlania miniatur zdjęć eksponatów w QGraphicsView.
 *
 * @section Structure
 * Plik nagłówkowy zawiera:
 * 1. **Deklarację klasy PhotoItem** – dziedziczy po QObject i QGraphicsPixmapItem.
 * 2. **Metody publiczne** – konstruktor, destruktor, ustawianie zaznaczenia.
 * 3. **Sygnały** – emitowane dla kliknięć, podwójnych kliknięć i zdarzeń hover.
 * 4. **Metody chronione** – obsługa zdarzeń myszy i hover.
 * 5. **Metody prywatne** – aktualizacja ramki zaznaczenia.
 * 6. **Zmienne prywatne** – flagi stanu i ramka graficzna.
 *
 * @section Dependencies
 * - **Qt Framework**: Używa klas QGraphicsPixmapItem, QObject, QGraphicsRectItem,
 *   QGraphicsSceneHoverEvent, QGraphicsSceneMouseEvent.
 *
 * @section Notes
 * - Kod nie został zmodyfikowany, zgodnie z wymaganiami użytkownika. Dodano jedynie komentarze i dokumentację.
 * - Klasa jest kluczowym komponentem do interaktywnego wyświetlania miniatur zdjęć w MainWindow.
 * - Sygnały umożliwiają integrację z MainWindow dla podglądu zdjęć lub ich usuwania.
 */

#ifndef PHOTOITEM_H
#define PHOTOITEM_H

#include <QGraphicsPixmapItem>
#include <QGraphicsRectItem>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <QObject>

/**
 * @class PhotoItem
 * @brief Element graficzny reprezentujący zdjęcie z obsługą zaznaczenia i zdarzeń interaktywnych.
 *
 * @section ClassOverview
 * Klasa PhotoItem dziedziczy po QGraphicsPixmapItem i QObject, co umożliwia
 * wyświetlanie grafiki na scenie graficznej Qt oraz emitowanie sygnałów Qt.
 * Umożliwia reagowanie na kliknięcia i zdarzenia kursora myszy (hover),
 * wizualne oznaczenie zaznaczenia oraz emitowanie sygnałów do logiki aplikacji,
 * np. MainWindow, dla podglądu zdjęć lub ich usuwania.
 *
 * @section Responsibilities
 * - Wyświetlanie miniatury zdjęcia w QGraphicsView.
 * - Obsługa kliknięć (pojedynczych i podwójnych) oraz zdarzeń hover.
 * - Wizualne zaznaczanie miniatury za pomocą czerwonej ramki.
 * - Emitowanie sygnałów dla interakcji użytkownika (clicked, doubleClicked, hovered, unhovered).
 */
class PhotoItem : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT

public:
    /**
     * @brief Konstruktor klasy PhotoItem.
     * @param parent Opcjonalny wskaźnik na element nadrzędny QGraphicsItem.
     *
     * @section ConstructorOverview
     * Inicjalizuje obiekt, ustawia domyślne właściwości (brak zaznaczenia, obsługa hover),
     * tworzy ramkę graficzną dla zaznaczenia i konfiguruje jej początkowy stan.
     */
    explicit PhotoItem(QGraphicsItem *parent = nullptr);

    /**
     * @brief Destruktor klasy PhotoItem.
     *
     * @section DestructorOverview
     * Usuwa dynamicznie alokowaną ramkę zaznaczenia (m_frame).
     */
    ~PhotoItem();

    /**
     * @brief Ustawia stan zaznaczenia obiektu.
     * @param selected Wartość true zaznacza element, false odznacza.
     *
     * @section MethodOverview
     * Aktualizuje stan zaznaczenia (m_selected) i wywołuje updateFrame()
     * w celu zmiany wyglądu ramki graficznej.
     */
    void setSelected(bool selected);

signals:
    /**
     * @brief Sygnał emitowany po jednokrotnym kliknięciu lewym przyciskiem myszy.
     * @param item Wskaźnik na kliknięty element PhotoItem.
     *
     * @section SignalOverview
     * Używany w MainWindow do wyboru miniatury zdjęcia w celu np. podglądu
     * lub przygotowania do usunięcia.
     */
    void clicked(PhotoItem *item);

    /**
     * @brief Sygnał emitowany po dwukrotnym kliknięciu lewym przyciskiem myszy.
     * @param item Wskaźnik na kliknięty element PhotoItem.
     *
     * @section SignalOverview
     * Używany do otwierania zdjęcia w trybie pełnoekranowym z możliwością
     * powiększania i przesuwania.
     */
    void doubleClicked(PhotoItem *item);

    /**
     * @brief Sygnał emitowany po najechaniu kursorem na obiekt.
     * @param item Wskaźnik do obiektu PhotoItem, na który najechano.
     *
     * @section SignalOverview
     * Może być używany do podświetlania miniatury lub wyświetlania informacji.
     */
    void hovered(PhotoItem *item);

    /**
     * @brief Sygnał emitowany po opuszczeniu kursora z obiektu.
     * @param item Wskaźnik do obiektu PhotoItem, który przestał być hoverowany.
     *
     * @section SignalOverview
     * Może być używany do usuwania podświetlenia miniatury.
     */
    void unhovered(PhotoItem *item);

protected:
    /**
     * @brief Obsługa zdarzenia kliknięcia myszy.
     * @param event Obiekt zawierający dane o zdarzeniu myszy.
     *
     * @section MethodOverview
     * Przechwytuje kliknięcia (prawy przycisk emituje clicked, lewy przekazywany
     * dla podwójnego kliknięcia) i przekazuje zdarzenie do QGraphicsPixmapItem.
     */
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

    /**
     * @brief Obsługa zdarzenia podwójnego kliknięcia myszy.
     * @param event Obiekt zawierający dane o zdarzeniu podwójnego kliknięcia.
     *
     * @section MethodOverview
     * Przechwytuje podwójne kliknięcia lewym przyciskiem, emituje sygnał doubleClicked
     * i przekazuje zdarzenie do QGraphicsPixmapItem.
     */
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

    /**
     * @brief Obsługa zdarzenia najechania kursora.
     * @param event Obiekt zawierający dane o zdarzeniu hover enter.
     *
     * @section MethodOverview
     * Emituje sygnał hovered i przekazuje zdarzenie do QGraphicsPixmapItem.
     */
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;

    /**
     * @brief Obsługa zdarzenia opuszczenia kursora.
     * @param event Obiekt zawierający dane o zdarzeniu hover leave.
     *
     * @section MethodOverview
     * Emituje sygnał unhovered i przekazuje zdarzenie do QGraphicsPixmapItem.
     */
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

private:
    /**
     * @brief Flaga oznaczająca, czy obiekt został kliknięty.
     *
     * @section VariableOverview
     * Obecnie nieużywana w implementacji, zachowana dla potencjalnych rozszerzeń.
     */
    bool m_pressed;

    /**
     * @brief Flaga oznaczająca, czy obiekt jest zaznaczony.
     *
     * @section VariableOverview
     * Kontroluje, czy ramka zaznaczenia (m_frame) jest wyświetlana.
     */
    bool m_selected;

    /**
     * @brief Ramka graficzna wykorzystywana do oznaczania zaznaczenia.
     *
     * @section VariableOverview
     * Obiekt QGraphicsRectItem wyświetlający czerwoną ramkę wokół miniatury
     * przy zaznaczeniu (m_selected == true).
     */
    QGraphicsRectItem *m_frame;

    /**
     * @brief Aktualizuje wygląd ramki zaznaczenia w zależności od stanu.
     *
     * @section MethodOverview
     * Ustawia czerwoną ramkę o szerokości 2 piksele, gdy obiekt jest zaznaczony,
     * lub ukrywa ramkę, gdy nie jest zaznaczony.
     */
    void updateFrame();
};

#endif // PHOTOITEM_H
