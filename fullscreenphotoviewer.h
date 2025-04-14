#ifndef FULLSCREENPHOTOVIEWER_H
#define FULLSCREENPHOTOVIEWER_H

#include <QMainWindow>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QKeyEvent>
#include <QWheelEvent>

/*
 * Klasa ZoomableGraphicsView – dziedziczy po QGraphicsView
 * i obsługuje zoom kółkiem myszy + tryb przesuwania obrazu (ScrollHandDrag).
 */
class ZoomableGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit ZoomableGraphicsView(QWidget *parent = nullptr)
        : QGraphicsView(parent), m_scaleFactor(1.0)
    {
        // Zoom względem miejsca pod wskaźnikiem myszy:
        setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
        // Umożliwiamy przesuwanie widoku przy powiększeniu:
        setDragMode(QGraphicsView::ScrollHandDrag);
    }

protected:
    void wheelEvent(QWheelEvent *event) override
    {
        // Zoom in/out:
        constexpr double zoomInFactor = 1.15;
        constexpr double zoomOutFactor = 1.0 / zoomInFactor;

        if (event->angleDelta().y() > 0) {
            // Kółko w górę => powiększ
            scale(zoomInFactor, zoomInFactor);
            m_scaleFactor *= zoomInFactor;
        } else {
            // Kółko w dół => pomniejsz
            scale(zoomOutFactor, zoomOutFactor);
            m_scaleFactor *= zoomOutFactor;
        }
        event->accept();
    }

private:
    double m_scaleFactor;
};

/*
 * Klasa FullScreenPhotoViewer – otwiera okno pełnoekranowe (bez ramki),
 * zawsze na wierzchu i modalne, z wyświetloną pixmapą w QGraphicsScene.
 * Obsługuje ESC do zamknięcia i zoom/pan w ZoomableGraphicsView.
 */
class FullScreenPhotoViewer : public QMainWindow
{
    Q_OBJECT
public:
    explicit FullScreenPhotoViewer(const QPixmap &pixmap, QWidget *parent = nullptr)
        : QMainWindow(parent)
    {
        // Ukrywamy ramkę okna i ustawiamy "zawsze na wierzchu":
        setWindowFlags(windowFlags()
                       | Qt::FramelessWindowHint
                       | Qt::WindowStaysOnTopHint);

        // Ustawiamy modalność aplikacji (blokuje kliknięcia w oknie pod spodem)
        setWindowModality(Qt::ApplicationModal);

        // Gdy zamkniemy okno, zostanie usunięte z pamięci
        setAttribute(Qt::WA_DeleteOnClose, true);

        // Tworzymy scenę i dodajemy do niej pixmapę
        m_scene = new QGraphicsScene(this);
        m_scene->addPixmap(pixmap);

        // Tworzymy nasz widok z zoomem i podpinamy scenę
        m_view = new ZoomableGraphicsView(this);
        m_view->setScene(m_scene);

        // Osadzamy widok jako centralny widget
        setCentralWidget(m_view);

        // Upewniamy się, że okno jest aktywne i na wierzchu
        activateWindow();
        raise();

        // Uruchamiamy w trybie pełnoekranowym
        showFullScreen();
    }

protected:
    void keyPressEvent(QKeyEvent *event) override
    {
        // ESC zamyka okno
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
