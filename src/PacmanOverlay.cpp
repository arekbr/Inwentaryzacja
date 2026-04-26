#include "PacmanOverlay.h"

#include <QDate>
#include <QImage>
#include <QLineEdit>
#include <QPainter>
#include <QPlainTextEdit>
#include <QRandomGenerator>
#include <QTextEdit>
#include <QTimerEvent>

#include <algorithm>
#include <cmath>

namespace {

constexpr int kAnimationTickMs = 16;
constexpr int kAnimationFrameMs = 40;
constexpr int kSpriteSizePx = 32;

QString targetWidgetText(QWidget *widget)
{
    if (auto lineEdit = qobject_cast<QLineEdit *>(widget))
        return lineEdit->text();
    if (auto textEdit = qobject_cast<QTextEdit *>(widget))
        return textEdit->toPlainText();
    if (auto plainTextEdit = qobject_cast<QPlainTextEdit *>(widget))
        return plainTextEdit->toPlainText();
    return QString();
}

int targetWidgetTextWidth(QWidget *widget)
{
    const QString text = targetWidgetText(widget);
    if (auto lineEdit = qobject_cast<QLineEdit *>(widget))
        return QFontMetrics(lineEdit->font()).horizontalAdvance(text);
    if (auto textEdit = qobject_cast<QTextEdit *>(widget))
        return QFontMetrics(textEdit->font()).horizontalAdvance(text);
    if (auto plainTextEdit = qobject_cast<QPlainTextEdit *>(widget))
        return QFontMetrics(plainTextEdit->font()).horizontalAdvance(text);
    return 0;
}

int targetWidgetApproxCharWidth(QWidget *widget)
{
    const QString text = targetWidgetText(widget);
    const QChar sampleChar = text.isEmpty() ? QLatin1Char(' ') : text.at(text.length() - 1);

    if (auto lineEdit = qobject_cast<QLineEdit *>(widget))
        return QFontMetrics(lineEdit->font()).horizontalAdvance(sampleChar);
    if (auto textEdit = qobject_cast<QTextEdit *>(widget))
        return QFontMetrics(textEdit->font()).horizontalAdvance(sampleChar);
    if (auto plainTextEdit = qobject_cast<QPlainTextEdit *>(widget))
        return QFontMetrics(plainTextEdit->font()).horizontalAdvance(sampleChar);
    return 0;
}

bool removeLastCharacter(QWidget *widget)
{
    if (auto lineEdit = qobject_cast<QLineEdit *>(widget)) {
        const QString text = lineEdit->text();
        if (text.isEmpty())
            return false;
        lineEdit->setText(text.left(text.length() - 1));
        return true;
    }

    if (auto textEdit = qobject_cast<QTextEdit *>(widget)) {
        const QString text = textEdit->toPlainText();
        if (text.isEmpty())
            return false;
        textEdit->setPlainText(text.left(text.length() - 1));
        return true;
    }

    if (auto plainTextEdit = qobject_cast<QPlainTextEdit *>(widget)) {
        const QString text = plainTextEdit->toPlainText();
        if (text.isEmpty())
            return false;
        plainTextEdit->setPlainText(text.left(text.length() - 1));
        return true;
    }

    return false;
}

}

PacmanOverlay::PacmanOverlay(QWidget *parent)
    : QWidget(parent),
      m_targetWidget(nullptr),
      m_timerId(0),
      m_durationMs(5000),
      m_pacmanFrame(0),
      m_showGhost(false),
      m_pacmanX(0.0),
      m_ghostX(0.0),
      m_pacmanCollided(false),
      m_collisionFrame(0),
      m_collisionHideMs(3000)
{
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
    setVisible(false);

    m_spriteSheet = QPixmap(QStringLiteral(":/images/spritesheet.png"));
    if (m_spriteSheet.isNull())
        qWarning() << "[PACMAN] Sprite sheet (spritesheet.png) not loaded!";
}

double PacmanOverlay::s_pacmanSpeedPx = 0.75;
int PacmanOverlay::s_eatCharIntervalMs = 100;
bool PacmanOverlay::s_enableBirthdayActivation = true;
int PacmanOverlay::s_birthdayDay = 11;
int PacmanOverlay::s_birthdayMonth = 12;
bool PacmanOverlay::s_enablePacManReleaseActivation = true;
bool PacmanOverlay::s_enableRandomActivation = true;
int PacmanOverlay::s_randomActivationChance = 10;
bool PacmanOverlay::s_enableCapacityActivation = true;
int PacmanOverlay::s_capacityCharCount = 22;

void PacmanOverlay::setTargetWidget(QWidget *target)
{
    m_targetWidget = target;
    if (!target)
        return;

    setParent(target);
    resize(target->size());
    move(0, 0);
    setVisible(false);
}

void PacmanOverlay::start(int durationMs)
{
    if (!m_targetWidget)
        return;

    const QString currentText = targetWidgetText(m_targetWidget);
    if (currentText.isEmpty())
        return;

    const bool capacityActivation = hasCapacityActivation(currentText);
    if (!capacityActivation) {
        const bool specialDateActive = isSpecialDate();
        bool randomChance = false;
        if (s_enableRandomActivation) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
            randomChance = QRandomGenerator::global()->bounded(100) < s_randomActivationChance;
#else
            randomChance = (qrand() % 100) < s_randomActivationChance;
#endif
        }

        if (!specialDateActive && !randomChance) {
            stopAnimation();
            return;
        }
    }

    stopAnimation();

    m_durationMs = durationMs;

    PacmanAnimationModel::Config config;
    config.activationDelayMs = m_durationMs;
    config.ghostDelayMs = 500;
    config.eatCharIntervalMs = s_eatCharIntervalMs;
    config.collisionFrameIntervalMs = kAnimationFrameMs;
    config.collisionHoldMs = m_collisionHideMs;
    config.pacmanSpeedPxPerSecond = s_pacmanSpeedPx * (1000.0 / kAnimationFrameMs);
    config.spriteSizePx = kSpriteSizePx;
    m_animationModel.configure(config);
    m_animationModel.start(targetWidgetTextWidth(m_targetWidget),
                           targetWidgetApproxCharWidth(m_targetWidget),
                           currentText.length());

    syncVisualState();
    m_frameClock.start();
    m_timerId = startTimer(kAnimationTickMs);
}

void PacmanOverlay::paintEvent(QPaintEvent *)
{
    if (!m_targetWidget || !m_animationModel.showOverlay())
        return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const int y = (height() - kSpriteSizePx) / 2;
    drawPacman(painter, (std::max)(static_cast<int>(std::round(m_pacmanX)), 0), y, kSpriteSizePx);
    if (m_showGhost)
        drawGhost(painter, (std::max)(static_cast<int>(std::round(m_ghostX)), 0), y, kSpriteSizePx);

    raise();
}

void PacmanOverlay::timerEvent(QTimerEvent *event)
{
    if (event->timerId() != m_timerId)
        return;

    const int deltaMs = std::clamp(static_cast<int>(m_frameClock.restart()), 1, 100);
    m_animationModel.advance(deltaMs);

    const int pendingEatCount = m_animationModel.takePendingEatCount();
    for (int index = 0; index < pendingEatCount; ++index) {
        if (!removeLastCharacter(m_targetWidget))
            break;
    }
    m_animationModel.setCurrentTextWidth(targetWidgetTextWidth(m_targetWidget));

    if (m_animationModel.consumeActivatedFlag()) {
        setVisible(true);
        emit activated();
    }

    syncVisualState();
    update();

    if (m_animationModel.consumeFinishedFlag()) {
        stopAnimation();
        emit finished();
        deleteLater();
    }
}

void PacmanOverlay::drawPacman(QPainter &painter, int x, int y, int size)
{
    if (m_spriteSheet.isNull())
        return;

    const int frameWidth = 32;
    const int frameHeight = 32;
    QRect sourceRect;

    if (m_pacmanCollided) {
        if (m_collisionFrame < 7)
            sourceRect = QRect(m_collisionFrame * frameWidth, 6 * frameHeight, frameWidth, frameHeight);
        else
            sourceRect = QRect((m_collisionFrame - 7) % 11 * frameWidth, 6 * frameHeight, frameWidth, frameHeight);
    } else {
        if (m_pacmanFrame % 3 == 0)
            sourceRect = QRect(4 * frameWidth, 0, frameWidth, frameHeight);
        else if (m_pacmanFrame % 3 == 1)
            sourceRect = QRect(0, 0, frameWidth, frameHeight);
        else
            sourceRect = QRect(0, frameHeight, frameWidth, frameHeight);
    }

    QImage frameImage = m_spriteSheet.copy(sourceRect).toImage().convertToFormat(QImage::Format_ARGB32);
    for (int yPixel = 0; yPixel < frameImage.height(); ++yPixel) {
        QRgb *line = reinterpret_cast<QRgb *>(frameImage.scanLine(yPixel));
        for (int xPixel = 0; xPixel < frameImage.width(); ++xPixel) {
            if (qRed(line[xPixel]) == 255 && qGreen(line[xPixel]) == 0 && qBlue(line[xPixel]) == 255)
                line[xPixel] = qRgba(255, 0, 255, 0);
        }
    }

    painter.drawImage(QRect(x, y, size, size), frameImage);
}

void PacmanOverlay::drawGhost(QPainter &painter, int x, int y, int size)
{
    if (m_spriteSheet.isNull())
        return;

    const int frameWidth = 32;
    const int frameHeight = 32;
    const QRect sourceRect(0, 4 * frameHeight, frameWidth, frameHeight);
    QImage frameImage = m_spriteSheet.copy(sourceRect).toImage().convertToFormat(QImage::Format_ARGB32);

    for (int yPixel = 0; yPixel < frameImage.height(); ++yPixel) {
        QRgb *line = reinterpret_cast<QRgb *>(frameImage.scanLine(yPixel));
        for (int xPixel = 0; xPixel < frameImage.width(); ++xPixel) {
            if (qRed(line[xPixel]) == 255 && qGreen(line[xPixel]) == 0 && qBlue(line[xPixel]) == 255)
                line[xPixel] = qRgba(255, 0, 255, 0);
        }
    }

    painter.drawImage(QRect(x, y, size, size), frameImage);
}

void PacmanOverlay::stopAnimation()
{
    if (m_timerId != 0) {
        killTimer(m_timerId);
        m_timerId = 0;
    }

    m_animationModel.reset();
    m_showGhost = false;
    m_pacmanFrame = 0;
    m_pacmanX = 0.0;
    m_ghostX = 0.0;
    m_pacmanCollided = false;
    m_collisionFrame = 0;
    setVisible(false);
    update();
}

void PacmanOverlay::syncVisualState()
{
    m_pacmanFrame = m_animationModel.pacmanFrame();
    m_pacmanX = m_animationModel.pacmanX();
    m_ghostX = m_animationModel.ghostX();
    m_showGhost = m_animationModel.showGhost();
    m_pacmanCollided = m_animationModel.isCollided();
    m_collisionFrame = m_animationModel.collisionFrame();
}

bool PacmanOverlay::isSpecialDate() const
{
    const QDate currentDate = QDate::currentDate();
    const bool isPacManReleaseDate = currentDate.day() == 22 && currentDate.month() == 5
                                      && s_enablePacManReleaseActivation;
    const bool isBirthday = currentDate.day() == s_birthdayDay && currentDate.month() == s_birthdayMonth
                            && s_enableBirthdayActivation;
    return isPacManReleaseDate || isBirthday;
}

bool PacmanOverlay::hasCapacityActivation(const QString &text) const
{
    if (!s_enableCapacityActivation)
        return false;
    return text.length() == s_capacityCharCount;
}
