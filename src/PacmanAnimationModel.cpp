#include "PacmanAnimationModel.h"

#include <algorithm>

void PacmanAnimationModel::configure(const Config &config)
{
    m_config = config;
}

void PacmanAnimationModel::start(int textWidth, int approximateCharWidth, int charCount)
{
    reset();

    m_state = State::Delay;
    m_currentTextWidth = textWidth;
    m_remainingChars = (std::max)(0, charCount);
    m_pacmanX = static_cast<double>(textWidth);
    m_targetPacmanX = static_cast<double>(textWidth);

    const int emptySpaceWidth = (std::max)(approximateCharWidth, 1) * (std::max)(charCount, 1);
    const double ghostStartX = static_cast<double>(textWidth + emptySpaceWidth);
    m_ghostX = ghostStartX;

    const int effectiveChaseTimeMs = (std::max)(1, m_remainingChars * m_config.eatCharIntervalMs - m_config.ghostDelayMs);
    const double ghostDistance = (std::max)(0.0, ghostStartX);
    m_config.ghostSpeedPxPerSecond = (ghostDistance * 1000.0) / effectiveChaseTimeMs;
}

void PacmanAnimationModel::reset()
{
    m_state = State::Idle;
    m_currentTextWidth = 0;
    m_remainingChars = 0;
    m_elapsedMs = 0;
    m_eatAccumulatorMs = 0;
    m_frameAccumulatorMs = 0;
    m_collisionAccumulatorMs = 0;
    m_holdElapsedMs = 0;
    m_pendingEatCount = 0;
    m_pacmanFrame = 0;
    m_collisionFrame = 0;
    m_pacmanX = 0.0;
    m_targetPacmanX = 0.0;
    m_ghostX = 0.0;
    m_ghostVisible = false;
    m_ghostChasing = false;
    m_activationTriggered = false;
    m_finishedTriggered = false;
}

void PacmanAnimationModel::advance(int deltaMs)
{
    if (deltaMs <= 0)
        return;

    switch (m_state)
    {
    case State::Idle:
    case State::Finished:
        return;
    case State::Delay:
        advanceDelay(deltaMs);
        break;
    case State::Eating:
        advanceEating(deltaMs);
        break;
    case State::Collision:
        advanceCollision(deltaMs);
        break;
    case State::Hold:
        advanceHold(deltaMs);
        break;
    }
}

void PacmanAnimationModel::setCurrentTextWidth(int textWidth)
{
    m_currentTextWidth = (std::max)(0, textWidth);
    m_targetPacmanX = static_cast<double>(m_currentTextWidth);
}

bool PacmanAnimationModel::isRunning() const
{
    return m_state != State::Idle && m_state != State::Finished;
}

bool PacmanAnimationModel::showOverlay() const
{
    return m_state != State::Idle && m_state != State::Delay && m_state != State::Finished;
}

bool PacmanAnimationModel::showGhost() const
{
    return m_ghostVisible;
}

bool PacmanAnimationModel::isCollided() const
{
    return m_state == State::Collision || m_state == State::Hold;
}

bool PacmanAnimationModel::consumeActivatedFlag()
{
    const bool value = m_activationTriggered;
    m_activationTriggered = false;
    return value;
}

bool PacmanAnimationModel::consumeFinishedFlag()
{
    const bool value = m_finishedTriggered;
    m_finishedTriggered = false;
    return value;
}

int PacmanAnimationModel::takePendingEatCount()
{
    const int count = m_pendingEatCount;
    m_pendingEatCount = 0;
    return count;
}

PacmanAnimationModel::State PacmanAnimationModel::state() const
{
    return m_state;
}

int PacmanAnimationModel::pacmanFrame() const
{
    return m_pacmanFrame;
}

int PacmanAnimationModel::collisionFrame() const
{
    return m_collisionFrame;
}

double PacmanAnimationModel::pacmanX() const
{
    return m_pacmanX;
}

double PacmanAnimationModel::ghostX() const
{
    return m_ghostX;
}

void PacmanAnimationModel::advanceDelay(int deltaMs)
{
    m_elapsedMs += deltaMs;
    if (m_elapsedMs < m_config.activationDelayMs)
        return;

    m_state = State::Eating;
    m_activationTriggered = true;
    m_elapsedMs = 0;
}

void PacmanAnimationModel::advanceEating(int deltaMs)
{
    m_elapsedMs += deltaMs;
    m_frameAccumulatorMs += deltaMs;
    m_eatAccumulatorMs += deltaMs;

    while (m_frameAccumulatorMs >= 120) {
        m_pacmanFrame = (m_pacmanFrame + 1) % 3;
        m_frameAccumulatorMs -= 120;
    }

    while (m_remainingChars > 0 && m_eatAccumulatorMs >= m_config.eatCharIntervalMs) {
        m_eatAccumulatorMs -= m_config.eatCharIntervalMs;
        ++m_pendingEatCount;
        --m_remainingChars;
    }

    updatePacmanPosition(deltaMs);

    if (!m_ghostVisible && m_elapsedMs >= m_config.ghostDelayMs) {
        m_ghostVisible = true;
        m_ghostChasing = true;
    }

    updateGhostPosition(deltaMs);
}

void PacmanAnimationModel::advanceCollision(int deltaMs)
{
    m_collisionAccumulatorMs += deltaMs;
    while (m_collisionAccumulatorMs >= m_config.collisionFrameIntervalMs
           && m_collisionFrame < 17) {
        ++m_collisionFrame;
        m_collisionAccumulatorMs -= m_config.collisionFrameIntervalMs;
    }

    if (m_collisionFrame >= 17) {
        m_state = State::Hold;
        m_holdElapsedMs = 0;
    }
}

void PacmanAnimationModel::advanceHold(int deltaMs)
{
    m_holdElapsedMs += deltaMs;
    if (m_holdElapsedMs >= m_config.collisionHoldMs)
        enterFinished();
}

void PacmanAnimationModel::updatePacmanPosition(int deltaMs)
{
    const double step = m_config.pacmanSpeedPxPerSecond * (static_cast<double>(deltaMs) / 1000.0);
    if (m_pacmanX > m_targetPacmanX)
        m_pacmanX = (std::max)(m_targetPacmanX, m_pacmanX - step);
    else if (m_pacmanX < m_targetPacmanX)
        m_pacmanX = (std::min)(m_targetPacmanX, m_pacmanX + step);
}

void PacmanAnimationModel::updateGhostPosition(int deltaMs)
{
    if (!m_ghostChasing)
        return;

    const double step = m_config.ghostSpeedPxPerSecond * (static_cast<double>(deltaMs) / 1000.0);
    const double nextGhostX = m_ghostX - step;
    if (nextGhostX <= m_pacmanX + m_config.spriteSizePx) {
        m_ghostX = m_pacmanX + m_config.spriteSizePx;
        m_ghostChasing = false;
        m_state = State::Collision;
        m_collisionFrame = 0;
        m_collisionAccumulatorMs = 0;
        return;
    }

    m_ghostX = (std::max)(0.0, nextGhostX);
}

void PacmanAnimationModel::enterFinished()
{
    m_state = State::Finished;
    m_finishedTriggered = true;
    m_ghostVisible = false;
    m_ghostChasing = false;
}
