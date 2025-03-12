#include "photoitem.h"
#include <QPainter>

PhotoItem::PhotoItem(QGraphicsItem *parent)
    : QGraphicsItem(parent), m_pixmap()
{
    setFlag(QGraphicsItem::ItemIsSelectable, true);
}

PhotoItem::PhotoItem(const QPixmap &pixmap, QGraphicsItem *parent)
    : QGraphicsItem(parent), m_pixmap(pixmap)
{
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);
}

QRectF PhotoItem::boundingRect() const
{
    return m_pixmap.isNull() ? QRectF() : QRectF(0, 0, m_pixmap.width(), m_pixmap.height());
}

void PhotoItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    if (!m_pixmap.isNull()) {
        painter->drawPixmap(0, 0, m_pixmap);
    }
}

QVariant PhotoItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemSelectedHasChanged) {
        update();
    }
    return QGraphicsItem::itemChange(change, value);
}

void PhotoItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    emit clicked(this); // Emitujemy sygnał, gdy klikniemy na PhotoItem
    QGraphicsItem::mousePressEvent(event);
}
