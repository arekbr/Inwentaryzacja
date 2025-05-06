/**
 * @file photoitem.cpp
 * @brief Implementacja klasy PhotoItem, interaktywnego elementu graficznego dla zdjęć.
 * @author Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK & ChatGPT & GROK
 * @version 1.2.2
 * @date 2025-05-03
 *
 * @section Overview
 * Plik zawiera implementację metod klasy PhotoItem, która reprezentuje miniaturę zdjęcia
 * w scenie graficznej Qt (QGraphicsView). Klasa rozszerza QGraphicsPixmapItem o obsługę
 * zdarzeń myszy (kliknięcia, podwójne kliknięcia) i hover (najechanie, opuszczenie),
 * a także wizualne zaznaczanie za pomocą czerwonej ramki. Dzięki dziedziczeniu po QObject
 * umożliwia emisję sygnałów (clicked, doubleClicked, hovered, unhovered) do integracji
 * z logiką aplikacji, np. MainWindow w aplikacji inwentaryzacyjnej.
 *
 * @section Structure
 * Kod jest podzielony na następujące sekcje:
 * 1. **Konstruktor** – inicjalizuje obiekt, ramkę i zdarzenia.
 * 2. **Destruktor** – zwalnia zasoby ramki.
 * 3. **Metody publiczne** – ustawianie stanu zaznaczenia.
 * 4. **Metody chronione** – obsługa zdarzeń myszy i hover.
 * 5. **Metody prywatne** – aktualizacja ramki zaznaczenia.
 *
 * @section Dependencies
 * - **Qt Framework**: Używa klas QGraphicsSceneMouseEvent, QPainter, QPen, QUuid.
 * - **Nagłówki aplikacji**: photoitem.h.
 *
 * @section Notes
 * - Kod nie został zmodyfikowany, zgodnie z wymaganiami użytkownika. Dodano jedynie komentarze i dokumentację.
 * - Klasa jest używana w MainWindow do wyświetlania i interakcji z miniaturami zdjęć eksponatów.
 * - Flaga m_pressed jest obecnie nieużywana, ale zachowana dla potencjalnych rozszerzeń.
 */

#include "photoitem.h"
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QPen>
#include <QUuid>

/**
 * @brief Konstruktor klasy PhotoItem.
 * @param parent Wskaźnik na opcjonalny element nadrzędny w hierarchii QGraphicsItem.
 *
 * @section ConstructorOverview
 * Inicjalizuje obiekt jako QObject i QGraphicsPixmapItem, ustawia flagi
 * m_pressed i m_selected na false, tworzy ramkę graficzną (m_frame) jako dziecko,
 * włącza obsługę zdarzeń hover i ustawia początkowy stan ramki (niewidoczna).
 */
PhotoItem::PhotoItem(QGraphicsItem *parent)
    : QObject(nullptr)
    , QGraphicsPixmapItem(parent)
    , m_pressed(false)
    , m_selected(false)
    , m_frame(new QGraphicsRectItem(this))
    , m_isEditMode(false) // Inicjalizacja nowej flagi
{
    setAcceptHoverEvents(true);
    setAcceptTouchEvents(false);
    m_frame->setPen(QPen(Qt::NoPen));
    m_frame->setBrush(Qt::NoBrush);
    m_frame->setZValue(1);
    updateFrame();
}

/**
 * @brief Destruktor klasy PhotoItem.
 *
 * @section DestructorOverview
 * Usuwa dynamicznie alokowaną ramkę graficzną (m_frame).
 */
PhotoItem::~PhotoItem()
{
    delete m_frame;
}

/**
 * @brief Obsługuje kliknięcie myszy na obiekt.
 * @param event Zdarzenie myszy typu QGraphicsSceneMouseEvent.
 *
 * @section MethodOverview
 * Emituje sygnał clicked dla prawego przycisku myszy (np. do podglądu zdjęcia).
 * Lewy przycisk jest przekazywany do QGraphicsPixmapItem, aby umożliwić
 * wykrycie podwójnego kliknięcia. Inne przyciski są obsługiwane domyślnie.
 */
void PhotoItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (m_isEditMode) {
            // W trybie edycji w MainWindow emituj sygnał clicked dla lewego przycisku
            emit clicked(this);
        }
        // Przekaż zdarzenie do QGraphicsPixmapItem dla podwójnego kliknięcia
        QGraphicsPixmapItem::mousePressEvent(event);
    } else if (event->button() == Qt::RightButton) {
        // Zachowaj obecne zachowanie dla prawego przycisku (dla innych kontekstów)
        emit clicked(this);
    } else {
        QGraphicsPixmapItem::mousePressEvent(event);
    }
}

/**
 * @brief Obsługuje podwójne kliknięcie lewym przyciskiem myszy na obiekt.
 * @param event Zdarzenie myszy typu QGraphicsSceneMouseEvent.
 *
 * @section MethodOverview
 * Emituje sygnał doubleClicked dla lewego przycisku (np. do pełnoekranowego
 * podglądu zdjęcia). Inne przyciski są obsługiwane przez QGraphicsPixmapItem.
 */
void PhotoItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit doubleClicked(this);
    } else {
        QGraphicsPixmapItem::mouseDoubleClickEvent(event);
    }
}

/**
 * @brief Ustawia flagę zaznaczenia obiektu i aktualizuje jego ramkę.
 * @param selected Jeśli true — obiekt będzie wizualnie zaznaczony ramką.
 *
 * @section MethodOverview
 * Aktualizuje stan m_selected i wywołuje updateFrame, aby wyświetlić
 * lub ukryć czerwoną ramkę wokół miniatury.
 */
void PhotoItem::setSelected(bool selected)
{
    m_selected = selected;
    updateFrame();
}

/**
 * @brief Aktualizuje wygląd ramki zaznaczenia w zależności od stanu zaznaczenia.
 *
 * @section MethodOverview
 * Jeśli m_selected jest true, ustawia czerwoną ramkę o szerokości 2 piksele
 * wokół miniatury (z marginesem -2, +2 piksele). W przeciwnym razie ukrywa ramkę
 * poprzez ustawienie Qt::NoPen.
 */
void PhotoItem::updateFrame()
{
    if (m_selected) {
        m_frame->setRect(boundingRect().adjusted(-2, -2, 2, 2));
        m_frame->setPen(QPen(Qt::red, 2));
    } else {
        m_frame->setPen(QPen(Qt::NoPen));
    }
}

/**
 * @brief Obsługuje zdarzenie najechania kursora myszy na obiekt.
 * @param event Zdarzenie typu QGraphicsSceneHoverEvent.
 *
 * @section MethodOverview
 * Emituje sygnał hovered (np. do podświetlenia miniatury) i przekazuje
 * zdarzenie do QGraphicsPixmapItem.
 */
void PhotoItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    emit hovered(this);
    QGraphicsPixmapItem::hoverEnterEvent(event);
}

// Tryb edycji – pozwala MainWindow przechwytywać pojedynczy lewy klik,
// żeby np. zaznaczyć miniaturę do usunięcia. Poza tym nic nie zmienia.
void PhotoItem::setEditMode(bool editMode)
{
    m_isEditMode = editMode;
    // Jeśli chcesz wizualnie oznaczyć edytowalne miniatury,
    // możesz np. zmienić kursor albo ramkę – tu zostawiamy neutralnie.
}

/**
 * @brief Obsługuje zdarzenie opuszczenia kursora z obiektu.
 * @param event Zdarzenie typu QGraphicsSceneHoverEvent.
 *
 * @section MethodOverview
 * Emituje sygnał unhovered (np. do usunięcia podświetlenia) i przekazuje
 * zdarzenie do QGraphicsPixmapItem.
 */
void PhotoItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    emit unhovered(this);
    QGraphicsPixmapItem::hoverLeaveEvent(event);
}
