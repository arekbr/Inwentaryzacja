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
    int m_mouthDelta;      // Dodane: kąt otwarcia szczęki
    bool m_mouthClosing;   // Dodane: kierunek animacji szczęki

    // --- Sprite-based animation ---
    QPixmap m_spriteSheet;
    int m_pacmanFrame;
    int m_ghostFrame;
    int m_vanishFrame;
    bool m_pacmanVanishing;
    void drawPacman(QPainter &p, int x, int y, int size);
    void drawGhost(QPainter &p, int x, int y, int size);
    void drawPacmanVanish(QPainter &p, int x, int y, int size);

    // --- New variables for position animation ---
    int m_pacmanX;             // Pozycja Pac-Mana (przesunięcie w lewo)
    int m_ghostX;              // Pozycja ducha (przesunięcie w lewo)
    bool m_ghostChasing;       // Czy duch już goni Pac-Mana
    double m_ghostSpeedPx;     // Prędkość ducha w px/klatkę
    bool m_pacmanCollided;     // Stan kolizji z duchem
    int m_collisionFrame;      // Numer klatki animacji kolizji (0-10)
    int m_collisionHideMs;     // czas ukrycia po kolizji (ms)
    bool m_holdingLastFrame;   // Stan zatrzymania na ostatniej klatce
    int m_lastFrameHoldTimeMs; // Czas zatrzymania na ostatniej klatce (ms)

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
    // Prywatna metoda sprawdzająca, czy dzisiejsza data jest specjalną datą
    bool isSpecialDate() const;

    // Prywatna metoda sprawdzająca, czy tekst ma wymaganą długość do aktywacji
    bool hasCapacityActivation(const QString &text) const;
};
