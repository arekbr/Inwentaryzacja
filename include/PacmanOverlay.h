#pragma once
#include <QWidget>
#include <QTimer>
#include <QPixmap>

class PacmanOverlay : public QWidget
{
    Q_OBJECT
public:
    explicit PacmanOverlay(QWidget *parent = nullptr);
    void setTargetWidget(QWidget *target);
    void start(int durationMs = 5000);

signals:
    void finished();

protected:
    void paintEvent(QPaintEvent *) override;
    void timerEvent(QTimerEvent *event) override;

private:
    QWidget *m_targetWidget;
    int m_angle;
    int m_timerId;
    int m_eatCharTimerId; // nowy timer do zjadania znaków
    int m_durationMs;
    bool m_showGhost;
    QTimer m_lifetimeTimer;
    QString m_initialText; // Dodane: do przechowywania początkowego tekstu
    int m_mouthDelta; // Dodane: kąt otwarcia szczęki
    bool m_mouthClosing; // Dodane: kierunek animacji szczęki

    // --- Sprite-based animation ---
    QPixmap m_spriteSheet;
    int m_pacmanFrame;
    int m_ghostFrame;
    int m_vanishFrame;
    bool m_pacmanVanishing;
    void drawPacman(QPainter &p, int x, int y, int size);
    void drawGhost(QPainter &p, int x, int y, int size);
    void drawPacmanVanish(QPainter &p, int x, int y, int size);
};
