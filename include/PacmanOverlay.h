#pragma once
#include <QWidget>
#include <QElapsedTimer>
#include <QPixmap>

#include "PacmanAnimationModel.h"

class PacmanOverlay : public QWidget
{
    Q_OBJECT
public:
    explicit PacmanOverlay(QWidget *parent = nullptr);
    void setTargetWidget(QWidget *target);
    void start(int durationMs = 5000);

    static void setPacmanSpeedPx(double px) { s_pacmanSpeedPx = px; }
    static double getPacmanSpeedPx() { return s_pacmanSpeedPx; }
    static void setEatCharIntervalMs(int ms) { s_eatCharIntervalMs = ms; }
    static int getEatCharIntervalMs() { return s_eatCharIntervalMs; }

    void setCollisionHideMs(int ms) { m_collisionHideMs = ms; }
    int getCollisionHideMs() const { return m_collisionHideMs; }

    // Metody do obsługi specjalnych dat aktywacji
    static void setBirthdayActivation(int day, int month)
    {
        s_birthdayDay = day;
        s_birthdayMonth = month;
    }
    static bool isBirthdayActivationEnabled() { return s_enableBirthdayActivation; }
    static void enableBirthdayActivation(bool enable) { s_enableBirthdayActivation = enable; }
    static bool isPacManReleaseActivationEnabled() { return s_enablePacManReleaseActivation; }
    static void enablePacManReleaseActivation(bool enable) { s_enablePacManReleaseActivation = enable; }
    static bool isRandomActivationEnabled() { return s_enableRandomActivation; }
    static void enableRandomActivation(bool enable) { s_enableRandomActivation = enable; }
    static int getRandomActivationChance() { return s_randomActivationChance; }
    static void setRandomActivationChance(int chance) { s_randomActivationChance = qBound(0, chance, 100); }

    // Metody do obsługi aktywacji opartej na dokładnej długości tekstu
    static bool isCapacityActivationEnabled() { return s_enableCapacityActivation; }
    static void enableCapacityActivation(bool enable) { s_enableCapacityActivation = enable; }
    static int getCapacityCharCount() { return s_capacityCharCount; }
    static void setCapacityCharCount(int count) { s_capacityCharCount = qBound(1, count, 1000); }

signals:
    void activated();
    void finished();

protected:
    void paintEvent(QPaintEvent *) override;
    void timerEvent(QTimerEvent *event) override;

private:
    QWidget *m_targetWidget;
    int m_timerId;
    int m_durationMs;
    QElapsedTimer m_frameClock;
    PacmanAnimationModel m_animationModel;

    // --- Sprite-based animation ---
    QPixmap m_spriteSheet;
    int m_pacmanFrame;
    void drawPacman(QPainter &p, int x, int y, int size);
    void drawGhost(QPainter &p, int x, int y, int size);

    bool m_showGhost;
    double m_pacmanX;
    double m_ghostX;
    bool m_pacmanCollided;
    int m_collisionFrame;
    int m_collisionHideMs;

    static double s_pacmanSpeedPx;
    static int s_eatCharIntervalMs;

    // Zmienne do obsługi specjalnych dat aktywacji
    static bool s_enableBirthdayActivation;      // Czy aktywować easter egg w dniu urodzin
    static int s_birthdayDay;                    // Dzień urodzin (11)
    static int s_birthdayMonth;                  // Miesiąc urodzin (12 - grudzień)
    static bool s_enablePacManReleaseActivation; // Czy aktywować easter egg w rocznicę premiery Pac-Mana
    static bool s_enableRandomActivation;        // Czy włączyć losową aktywację
    static int s_randomActivationChance;         // Szansa na losową aktywację (0-100%)
    static bool s_enableCapacityActivation;      // Czy włączyć aktywację opartą na długości tekstu
    static int s_capacityCharCount;              // Dokładna liczba znaków potrzebna do aktywacji (domyślnie 22)

private:
    void stopAnimation();
    void syncVisualState();

    // Prywatna metoda sprawdzająca, czy dzisiejsza data jest specjalną datą
    bool isSpecialDate() const;

    // Prywatna metoda sprawdzająca, czy tekst ma wymaganą długość do aktywacji
    bool hasCapacityActivation(const QString &text) const;
};
