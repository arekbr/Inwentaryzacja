/**
 * @file fullscreenphotoviewer.h
 * @brief Deklaracja klas ZoomableGraphicsView i FullScreenPhotoViewer do pełnoekranowego podglądu zdjęć.
 * @author Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK & ChatGPT & GROK
 * @version 1.1.8
 * @date 2025-04-25
 *
 * Plik zawiera deklaracje dwóch klas: ZoomableGraphicsView, która umożliwia powiększanie i przesuwanie
 * obrazu za pomocą kółka myszy i przeciągania, oraz FullScreenPhotoViewer, która wyświetla zdjęcie
 * w trybie pełnoekranowym z możliwością zamknięcia okna klawiszem Escape. Klasy są używane w aplikacji
 * inwentaryzacyjnej do podglądu zdjęć eksponatów.
 */

#ifndef FULLSCREENPHOTOVIEWER_H
#define FULLSCREENPHOTOVIEWER_H

#include <QMainWindow>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QKeyEvent>
#include <QWheelEvent>

/**
 * @class ZoomableGraphicsView
 * @brief Widok graficzny umożliwiający powiększanie i przesuwanie obrazu.
 *
 * Klasa ZoomableGraphicsView dziedziczy po QGraphicsView i implementuje funkcjonalność
 * powiększania obrazu za pomocą kółka myszy oraz przesuwania obrazu przez przeciąganie.
 * Ustawia kotwicę transformacji pod kursorem myszy i tryb przeciągania typu ScrollHandDrag.
 */
class ZoomableGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    /**
     * @brief Konstruktor klasy ZoomableGraphicsView.
     * @param parent Wskaźnik na nadrzędny widget. Domyślnie nullptr.
     *
     * Inicjalizuje widok graficzny z włączonym trybem przeciągania i kotwicą transformacji
     * pod kursorem myszy.
     */
    explicit ZoomableGraphicsView(QWidget *parent = nullptr)
        : QGraphicsView(parent), m_scaleFactor(1.0)
    {
        setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
        setDragMode(QGraphicsView::ScrollHandDrag);
    }

protected:
    /**
     * @brief Obsługuje zdarzenia kółka myszy do powiększania/pomniejszania obrazu.
     * @param event Wskaźnik na zdarzenie kółka myszy.
     *
     * Powiększa obraz o współczynnik 1.15 przy przewijaniu w górę i pomniejsza o odwrotność
     * tego współczynnika przy przewijaniu w dół. Aktualizuje bieżący współczynnik skali.
     */
    void wheelEvent(QWheelEvent *event) override
    {
        constexpr double zoomInFactor = 1.15;
        constexpr double zoomOutFactor = 1.0 / zoomInFactor;

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
    /// Bieżący współczynnik skali obrazu.
    double m_scaleFactor;
};

/**
 * @class FullScreenPhotoViewer
 * @brief Okno pełnoekranowe do wyświetlania zdjęć.
 *
 * Klasa FullScreenPhotoViewer dziedziczy po QMainWindow i wyświetla zdjęcie w trybie pełnoekranowym,
 * korzystając z ZoomableGraphicsView. Okno jest bezramkowe, pozostaje na wierzchu i zamyka się
 * po naciśnięciu klawisza Escape. Jest modalne i automatycznie usuwa się po zamknięciu.
 */
class FullScreenPhotoViewer : public QMainWindow
{
    Q_OBJECT
public:
    /**
     * @brief Konstruktor klasy FullScreenPhotoViewer.
     * @param pixmap Obiekt QPixmap zawierający zdjęcie do wyświetlenia.
     * @param parent Wskaźnik na nadrzędny widget. Domyślnie nullptr.
     *
     * Inicjalizuje okno pełnoekranowe, ustawia zdjęcie w scenie graficznej, konfiguruje
     * ZoomableGraphicsView jako centralny widget i wyświetla okno w trybie pełnoekranowym.
     */
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
    /**
     * @brief Obsługuje zdarzenia naciśnięcia klawiszy.
     * @param event Wskaźnik na zdarzenie klawiatury.
     *
     * Zamyka okno po naciśnięciu klawisza Escape. Inne zdarzenia są przekazywane do QMainWindow.
     */
    void keyPressEvent(QKeyEvent *event) override
    {
        if (event->key() == Qt::Key_Escape) {
            close();
        } else {
            QMainWindow::keyPressEvent(event);
        }
    }

private:
    /// Wskaźnik na scenę graficzną zawierającą zdjęcie.
    QGraphicsScene *m_scene;
    /// Wskaźnik na widok graficzny obsługujący powiększanie i przesuwanie.
    ZoomableGraphicsView *m_view;
};

#endif // FULLSCREENPHOTOVIEWER_H
