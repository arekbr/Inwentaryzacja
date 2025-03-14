#ifndef PHOTOITEM_H
#define PHOTOITEM_H

#include <QGraphicsPixmapItem>
#include <QObject>
#include <QGraphicsRectItem>

class PhotoItem : public QObject, public QGraphicsPixmapItem {
    Q_OBJECT
public:
    explicit PhotoItem(QGraphicsItem *parent = nullptr);
    ~PhotoItem();
    void setSelected(bool selected);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override; // NOWE: Obsługa najechania
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override; // NOWE: Obsługa opuszczenia

signals:
    void clicked(PhotoItem *item);
    void hovered(PhotoItem *item);   // NOWY sygnał
    void unhovered(PhotoItem *item); // NOWY sygnał

private:
    bool m_pressed;
    bool m_selected;
    QGraphicsRectItem *m_frame;
    void updateFrame();
};

#endif // PHOTOITEM_H
