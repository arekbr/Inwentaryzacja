#ifndef PHOTOITEM_H
#define PHOTOITEM_H

#include <QGraphicsPixmapItem>
#include <QObject>
#include <QGraphicsRectItem>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>

class PhotoItem : public QObject, public QGraphicsPixmapItem {
    Q_OBJECT
public:
    explicit PhotoItem(QGraphicsItem *parent = nullptr);
    ~PhotoItem();
    void setSelected(bool selected);

signals:
    void clicked(PhotoItem *item);
    void hovered(PhotoItem *item);
    void unhovered(PhotoItem *item);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

private:
    bool m_pressed;
    bool m_selected;
    QGraphicsRectItem *m_frame;
    void updateFrame();
};

#endif // PHOTOITEM_H
