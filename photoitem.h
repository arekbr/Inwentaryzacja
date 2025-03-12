#ifndef PHOTOITEM_H
#define PHOTOITEM_H

#include <QGraphicsItem>
#include <QPixmap>

class PhotoItem : public QGraphicsItem
{
public:
    explicit PhotoItem(QGraphicsItem *parent = nullptr);
    explicit PhotoItem(const QPixmap &pixmap, QGraphicsItem *parent = nullptr); // Poprawny konstruktor
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    QPixmap pixmap() const { return m_pixmap; }

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

signals:
    void clicked(PhotoItem *item);

private:
    QPixmap m_pixmap;
};

#endif // PHOTOITEM_H
