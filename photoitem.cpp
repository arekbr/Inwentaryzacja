#include "photoitem.h"
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QPen>

PhotoItem::PhotoItem(QGraphicsItem *parent)
    : QObject(nullptr),
    QGraphicsPixmapItem(parent),
    m_pressed(false),
    m_selected(false),
    m_frame(new QGraphicsRectItem(this))
{
    setAcceptHoverEvents(true); // Zmienione z false na true, aby włączyć zdarzenia hover
    setAcceptTouchEvents(false);
    m_frame->setPen(QPen(Qt::NoPen));
    m_frame->setBrush(Qt::NoBrush);
    m_frame->setZValue(1);
    updateFrame();
}

PhotoItem::~PhotoItem()
{
    delete m_frame;
}

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

void PhotoItem::setSelected(bool selected)
{
    m_selected = selected;
    updateFrame();
}

void PhotoItem::updateFrame()
{
    if (m_selected) {
        m_frame->setRect(boundingRect().adjusted(-2, -2, 2, 2));
        m_frame->setPen(QPen(Qt::red, 2));
    } else {
        m_frame->setPen(QPen(Qt::NoPen));
    }
}

void PhotoItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    emit hovered(this); // Emituj sygnał najechania
    QGraphicsPixmapItem::hoverEnterEvent(event);
}

void PhotoItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    emit unhovered(this); // Emituj sygnał opuszczenia
    QGraphicsPixmapItem::hoverLeaveEvent(event);
}
