/**
 * @file photoitem.cpp
 * @brief Implementacja klasy PhotoItem, interaktywnego elementu graficznego dla zdjęć w scenie Qt.
 * @author Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK
 * @author ChatGPT
 * @author GROK
 * @version 1.1.8
 * @date 2025-04-25
 *
 * Plik zawiera implementację metod klasy PhotoItem, umożliwiającej reprezentację zdjęcia w scenie graficznej.
 * Klasa rozszerza QGraphicsPixmapItem o obsługę zdarzeń myszy i hover oraz ramkę oznaczającą zaznaczenie.
 * Dzięki dziedziczeniu po QObject umożliwia emisję sygnałów typu clicked, hovered i unhovered.
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
 * Inicjalizuje obiekt, domyślnie ustawia brak zaznaczenia, tworzy ramkę graficzną (m_frame)
 * oraz włącza obsługę zdarzeń hover.
 */
PhotoItem::PhotoItem(QGraphicsItem *parent)
    : QObject(nullptr),
    QGraphicsPixmapItem(parent),
    m_pressed(false),
    m_selected(false),
    m_frame(new QGraphicsRectItem(this))
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
 * Usuwa ramkę graficzną `m_frame`, jeśli została utworzona.
 */
PhotoItem::~PhotoItem()
{
    delete m_frame;
}

/**
 * @brief Obsługuje kliknięcie lewym przyciskiem myszy na obiekt.
 * @param event Zdarzenie myszy typu QGraphicsSceneMouseEvent.
 *
 * Ustawia flagę `m_pressed` i emituje sygnał `clicked()`.
 * W przypadku innego przycisku myszy przekazuje zdarzenie do klasy bazowej.
 */
void PhotoItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_pressed = true;
        emit clicked(this);
        event->accept();
    } else {
        QGraphicsPixmapItem::mousePressEvent(event);
    }
}

/**
 * @brief Ustawia flagę zaznaczenia obiektu i aktualizuje jego ramkę.
 * @param selected Jeśli true — obiekt będzie wizualnie zaznaczony ramką.
 */
void PhotoItem::setSelected(bool selected)
{
    m_selected = selected;
    updateFrame();
}

/**
 * @brief Aktualizuje wygląd ramki zaznaczenia w zależności od stanu zaznaczenia.
 *
 * Jeśli obiekt jest zaznaczony (`m_selected == true`), ustawia czerwoną ramkę o szerokości 2 px.
 * W przeciwnym razie ramka jest ukrywana (NoPen).
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
 * Emisja sygnału `hovered()` oraz przekazanie zdarzenia do klasy bazowej.
 */
void PhotoItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    emit hovered(this);
    QGraphicsPixmapItem::hoverEnterEvent(event);
}

/**
 * @brief Obsługuje zdarzenie opuszczenia kursora z obiektu.
 * @param event Zdarzenie typu QGraphicsSceneHoverEvent.
 *
 * Emisja sygnału `unhovered()` oraz przekazanie zdarzenia do klasy bazowej.
 */
void PhotoItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    emit unhovered(this);
    QGraphicsPixmapItem::hoverLeaveEvent(event);
}
