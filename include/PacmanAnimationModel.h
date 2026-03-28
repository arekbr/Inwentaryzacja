#ifndef PACMANANIMATIONMODEL_H
#define PACMANANIMATIONMODEL_H

class PacmanAnimationModel
{
public:
    struct Config
    {
        int activationDelayMs = 5000;
        int ghostDelayMs = 500;
        int eatCharIntervalMs = 100;
        int collisionFrameIntervalMs = 40;
        int collisionHoldMs = 2000;
        double pacmanSpeedPxPerSecond = 18.75;
        double ghostSpeedPxPerSecond = 0.0;
        int spriteSizePx = 32;
    };

    enum class State
    {
        Idle,
        Delay,
        Eating,
        Collision,
        Hold,
        Finished
    };

    void configure(const Config &config);
    void start(int textWidth, int approximateCharWidth, int charCount);
    void reset();
    void advance(int deltaMs);
    void setCurrentTextWidth(int textWidth);

    bool isRunning() const;
    bool showOverlay() const;
    bool showGhost() const;
    bool isCollided() const;
    bool consumeActivatedFlag();
    bool consumeFinishedFlag();
    int takePendingEatCount();

    State state() const;
    int pacmanFrame() const;
    int collisionFrame() const;
    double pacmanX() const;
    double ghostX() const;

private:
    Config m_config;
    State m_state = State::Idle;
    int m_currentTextWidth = 0;
    int m_remainingChars = 0;
    int m_elapsedMs = 0;
    int m_eatAccumulatorMs = 0;
    int m_frameAccumulatorMs = 0;
    int m_collisionAccumulatorMs = 0;
    int m_holdElapsedMs = 0;
    int m_pendingEatCount = 0;
    int m_pacmanFrame = 0;
    int m_collisionFrame = 0;
    double m_pacmanX = 0.0;
    double m_targetPacmanX = 0.0;
    double m_ghostX = 0.0;
    bool m_ghostVisible = false;
    bool m_ghostChasing = false;
    bool m_activationTriggered = false;
    bool m_finishedTriggered = false;

    void advanceDelay(int deltaMs);
    void advanceEating(int deltaMs);
    void advanceCollision(int deltaMs);
    void advanceHold(int deltaMs);
    void updatePacmanPosition(int deltaMs);
    void updateGhostPosition(int deltaMs);
    void enterFinished();
};

#endif // PACMANANIMATIONMODEL_H
