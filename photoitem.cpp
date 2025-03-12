#include "photoitem.h"
#include <QGraphicsSceneMouseEvent>
#include <QPainter>

PhotoItem::PhotoItem(QGraphicsItem *parent) : QGraphicsPixmapItem(parent), m_pressed(false), m_selected(false), m_frame(nullptr)
{
    setAcceptHoverEvents(false);
    setAcceptTouchEvents(false);

    // Utwórz ramkę
    m_frame = new QGraphicsRectItem(this);
    m_frame->setPen(QPen(Qt::NoPen)); // Brak ramki domyślnie
    m_frame->setBrush(Qt::NoBrush);   // Brak wypełnienia
    m_frame->setZValue(1);            // Ramka na wierzchu
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
        m_frame->setRect(boundingRect().adjusted(-2, -2, 2, 2)); // Ramka o 2 piksele większa
        m_frame->setPen(QPen(Qt::red, 2)); // Czerwona ramka, 2 piksele
    } else {
        m_frame->setPen(QPen(Qt::NoPen)); // Brak ramki
    }
}
