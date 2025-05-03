/**
 * @file fullscreenphotoviewer.h
 * @brief Deklaracja klas ZoomableGraphicsView i FullScreenPhotoViewer do pełnoekranowego podglądu zdjęć.
 * @author Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK & ChatGPT & GROK
 * @version 1.2.2
 * @date 2025-05-03
 *
 * @section Overview
 * Plik fullscreenphotoviewer.h definiuje dwie klasy: ZoomableGraphicsView oraz FullScreenPhotoViewer, 
 * używane w aplikacji inwentaryzacyjnej do pełnoekranowego wyświetlania zdjęć eksponatów. Klasa 
 * ZoomableGraphicsView umożliwia powiększanie i przesuwanie obrazu za pomocą kółka myszy i przeciągania, 
 * natomiast FullScreenPhotoViewer tworzy bezramkowe okno pełnoekranowe z możliwością zamknięcia klawiszem Escape.
 *
 * @section Structure
 * Plik nagłówkowy zawiera:
 * 1. **Deklarację klasy ZoomableGraphicsView** – dziedziczy po QGraphicsView, implementuje powiększanie i przesuwanie.
 * 2. **Deklarację klasy FullScreenPhotoViewer** – dziedziczy po QMainWindow, zarządza pełnoekranowym wyświetlaniem zdjęcia.
 * 3. **Metody inline** – wszystkie metody są zdefiniowane w pliku nagłówkowym, eliminując potrzebę osobnego pliku .cpp.
 *
 * @section Dependencies
 * - **Qt Framework**: Używa klas QMainWindow, QGraphicsView, QGraphicsScene, QKeyEvent, QWheelEvent.
 * - **Zasoby**: Brak bezpośrednich zależności od zasobów Qt (np. plików QSS czy czcionek).
 *
 * @section Notes
 * - Kod nie został zmodyfikowany, zgodnie z wymaganiami użytkownika. Dodano jedynie komentarze i dokumentację.
 * - Obie klasy są zoptymalizowane pod kątem prostoty i wydajności, z implementacją inline w pliku nagłówkowym.
 * - FullScreenPhotoViewer jest modalny i automatycznie usuwa się po zamknięciu, co zapobiega wyciekom pamięci.
 */

#ifndef FULLSCREENPHOTOVIEWER_H
#define FULLSCREENPHOTOVIEWER_H

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QMainWindow>
#include <QWheelEvent>

/**
 * @class ZoomableGraphicsView
 * @brief Widok graficzny umożliwiający powiększanie i przesuwanie obrazu.
 *
 * @section ClassOverview
 * Klasa ZoomableGraphicsView dziedziczy po QGraphicsView i dostarcza funkcjonalność powiększania 
 * obrazu za pomocą kółka myszy oraz przesuwania przez przeciąganie. Ustawia kotwicę transformacji 
 * pod kursorem myszy, co zapewnia intuicyjne powiększanie, oraz tryb przeciągania ScrollHandDrag 
 * dla przesuwania obrazu.
 *
 * @section Responsibilities
 * - Obsługa zdarzeń kółka myszy do zmiany skali obrazu.
 * - Umożliwienie przesuwania obrazu poprzez przeciąganie myszą.
 * - Przechowywanie bieżącego współczynnika skali.
 */
class ZoomableGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    /**
     * @brief Konstruktor klasy ZoomableGraphicsView.
     * @param parent Wskaźnik na nadrzędny widget. Domyślnie nullptr.
     *
     * @section ConstructorOverview
     * Inicjalizuje widok graficzny, ustawia kotwicę transformacji pod kursorem myszy 
     * (AnchorUnderMouse) oraz tryb przeciągania (ScrollHandDrag). Ustawia początkowy 
     * współczynnik skali na 1.0.
     */
    explicit ZoomableGraphicsView(QWidget *parent = nullptr)
        : QGraphicsView(parent)
        , m_scaleFactor(1.0)
    {
        setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
        setDragMode(QGraphicsView::ScrollHandDrag);
    }

protected:
    /**
     * @brief Obsługuje zdarzenia kółka myszy do powiększania/pomniejszania obrazu.
     * @param event Wskaźnik na zdarzenie kółka myszy.
     *
     * @section MethodOverview
     * Przetwarza zdarzenia kółka myszy, zwiększając skalę obrazu o współczynnik 1.15 
     * (zoom in) lub zmniejszając o 1/1.15 (zoom out) w zależności od kierunku przewijania. 
     * Aktualizuje zmienną m_scaleFactor i akceptuje zdarzenie, aby zapobiec jego dalszemu 
     * przetwarzaniu.
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
 * @section ClassOverview
 * Klasa FullScreenPhotoViewer dziedziczy po QMainWindow i tworzy bezramkowe, modalne okno 
 * pełnoekranowe do wyświetlania zdjęcia w obiekcie QPixmap. Wykorzystuje ZoomableGraphicsView 
 * do obsługi powiększania i przesuwania obrazu. Okno zamyka się po naciśnięciu klawisza Escape 
 * i automatycznie usuwa się po zamknięciu (WA_DeleteOnClose).
 *
 * @section Responsibilities
 * - Wyświetlanie zdjęcia w trybie pełnoekranowym.
 * - Obsługa zamykania okna klawiszem Escape.
 * - Zarządzanie sceną graficzną i widokiem ZoomableGraphicsView.
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
     * @section ConstructorOverview
     * Inicjalizuje bezramkowe okno pełnoekranowe z flagami FramelessWindowHint i 
     * WindowStaysOnTopHint. Ustawia modalność (ApplicationModal) i automatyczne usuwanie 
     * po zamknięciu (WA_DeleteOnClose). Tworzy scenę graficzną, dodaje do niej zdjęcie, 
     * konfiguruje ZoomableGraphicsView jako centralny widget i wyświetla okno w trybie pełnoekranowym.
     */
    explicit FullScreenPhotoViewer(const QPixmap &pixmap, QWidget *parent = nullptr)
        : QMainWindow(parent)
    {
        setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

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
     * @section MethodOverview
     * Zamyka okno po naciśnięciu klawisza Escape, wywołując metodę close(). Dla innych 
     * klawiszy przekazuje zdarzenie do metody bazowej QMainWindow::keyPressEvent.
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
