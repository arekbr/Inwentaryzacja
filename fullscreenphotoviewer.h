#ifndef FULLSCREENPHOTOVIEWER_H
#define FULLSCREENPHOTOVIEWER_H

#include <QMainWindow>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QKeyEvent>
#include <QWheelEvent>

class ZoomableGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit ZoomableGraphicsView(QWidget *parent = nullptr)
        : QGraphicsView(parent), m_scaleFactor(1.0)
    {
        setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
        setDragMode(QGraphicsView::ScrollHandDrag);
    }

protected:
    void wheelEvent(QWheelEvent *event) override
    {
        constexpr double zoomInFactor = 1.15;
        constexpr double zoomOutFactor= 1.0/zoomInFactor;

        if (event->angleDelta().y() > 0) {
            scale(zoomInFactor, zoomInFactor);
            m_scaleFactor *= zoomInFactor;
        } else {
            scale(zoomOutFactor, zoomOutFactor);
            m_scaleFactor *= zoomOutFactor;
        }
        event->accept();
    }

private:
    double m_scaleFactor;
};

class FullScreenPhotoViewer : public QMainWindow
{
    Q_OBJECT
public:
    explicit FullScreenPhotoViewer(const QPixmap &pixmap, QWidget *parent = nullptr)
        : QMainWindow(parent)
    {
        setWindowFlags(windowFlags()
                       | Qt::FramelessWindowHint
                       | Qt::WindowStaysOnTopHint);

        setWindowModality(Qt::ApplicationModal);
        setAttribute(Qt::WA_DeleteOnClose, true);

        m_scene = new QGraphicsScene(this);
        m_scene->addPixmap(pixmap);

        m_view = new ZoomableGraphicsView(this);
        m_view->setScene(m_scene);

        setCentralWidget(m_view);

        activateWindow();
        raise();
        showFullScreen();
    }

protected:
    void keyPressEvent(QKeyEvent *event) override
    {
        if (event->key() == Qt::Key_Escape) {
            close();
        } else {
            QMainWindow::keyPressEvent(event);
        }
    }

private:
    QGraphicsScene *m_scene;
    ZoomableGraphicsView *m_view;
};

#endif // FULLSCREENPHOTOVIEWER_H
