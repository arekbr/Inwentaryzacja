#include "PacmanOverlay.h"
#include <QPainter>
#include <QLineEdit>
#include <QTextEdit>
#include <QTimerEvent>
#include <QPainterPath>
#include <QImage>

PacmanOverlay::PacmanOverlay(QWidget *parent)
    : QWidget(parent), m_targetWidget(nullptr), m_angle(0), m_timerId(0), m_durationMs(5000), m_showGhost(false),
      m_pacmanFrame(0), m_ghostFrame(0), m_vanishFrame(0), m_pacmanVanishing(false)
{
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
    setVisible(false);

    // Ładuj nowy sprite sheet
    m_spriteSheet = QPixmap(":/images/spritesheet.png");
    if (m_spriteSheet.isNull())
    {
        qWarning() << "[PACMAN] Sprite sheet (spritesheet.png) not loaded!";
    }

    connect(&m_lifetimeTimer, &QTimer::timeout, this, [this]()
            {
        m_showGhost = true;
        m_pacmanVanishing = true;
        m_vanishFrame = 0;
        update();
        QTimer::singleShot(700, this, [this]() {
            setVisible(false);
            emit finished();
        }); });

    // --- Dodane zmienne do animacji położenia Pac-Mana i ducha ---
    m_pacmanX = 0;
    m_ghostX = 0;
    m_ghostChasing = false;
}

// --- Statyczne zmienne do regulacji prędkości ---
int PacmanOverlay::s_pacmanSpeedPx = 0.75;    // domyślnie 3 px na klatkę
int PacmanOverlay::s_eatCharIntervalMs = 100; // domyślnie 500 ms na znak

void PacmanOverlay::setTargetWidget(QWidget *target)
{
    qDebug() << "[PACMAN] setTargetWidget: target=" << target << ", parent before=" << parent();
    m_targetWidget = target;
    if (target)
    {
        setParent(target);
        resize(target->size());
        move(0, 0);
        show();
        raise();
        qDebug() << "[PACMAN] setTargetWidget: parent after=" << parent() << ", geometry=" << geometry();
    }
}

void PacmanOverlay::start(int durationMs)
{
    if (!m_targetWidget)
        return;
    m_durationMs = durationMs;
    m_showGhost = false;
    setVisible(true);
    m_angle = 0;
    m_mouthDelta = 0;
    m_mouthClosing = false;
    m_timerId = startTimer(40);                         // timer do animacji szczęki
    m_eatCharTimerId = startTimer(s_eatCharIntervalMs); // timer do zjadania znaków (regulowany)
    m_lifetimeTimer.start(durationMs);
    m_pacmanX = 0;
    m_ghostX = 0;
    m_ghostChasing = false;
    if (auto le = qobject_cast<QLineEdit *>(m_targetWidget))
    {
        m_initialText = le->text();
    }
    else if (auto te = qobject_cast<QTextEdit *>(m_targetWidget))
    {
        m_initialText = te->toPlainText();
    }
    else
    {
        m_initialText.clear();
    }
    // Duch pojawia się po 2 sekundach na prawej krawędzi pola tekstowego
    QTimer::singleShot(2000, this, [this]() {
        m_showGhost = true;
        int size = 32;
        int textWidth = 0;
        if (auto le = qobject_cast<QLineEdit *>(m_targetWidget))
        {
            QFontMetrics fm(le->font());
            textWidth = fm.horizontalAdvance(le->text());
        }
        else if (auto te = qobject_cast<QTextEdit *>(m_targetWidget))
        {
            QFontMetrics fm(te->font());
            textWidth = fm.horizontalAdvance(te->toPlainText());
        }
        int startX = textWidth;
        // Duch na prawej krawędzi pola (nie na tekście, tylko na końcu pola)
        m_ghostX = -(width() - startX - size); // startX - m_ghostX = width() - size
        update();
    });
}

void PacmanOverlay::paintEvent(QPaintEvent *)
{
    if (!m_targetWidget)
    {
        qDebug() << "[PACMAN] paintEvent: m_targetWidget is nullptr, nothing to draw";
        return;
    }
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    int size = 32;
    int y = (height() - size) / 2;
    int textWidth = 0;
    if (auto le = qobject_cast<QLineEdit *>(m_targetWidget))
    {
        QFontMetrics fm(le->font());
        textWidth = fm.horizontalAdvance(le->text());
    }
    else if (auto te = qobject_cast<QTextEdit *>(m_targetWidget))
    {
        QFontMetrics fm(te->font());
        textWidth = fm.horizontalAdvance(te->toPlainText());
    }
    int startX = textWidth;
    int ghostMargin = 4; // odstęp od prawej krawędzi pola
    int ghostX = width() - size - ghostMargin;
    if (!m_showGhost)
    {
        drawPacman(p, startX - m_pacmanX, y, size);
    }
    else if (m_pacmanVanishing)
    {
        drawPacmanVanish(p, startX - m_pacmanX, y, size);
        drawGhost(p, ghostX, y, size); // Duch przy prawej krawędzi pola
    }
    else
    {
        drawGhost(p, ghostX, y, size); // Duch przy prawej krawędzi pola
    }
    raise();
}

// Helper: Draw Pac-Man using sprite sheet
void PacmanOverlay::drawPacman(QPainter &p, int x, int y, int size)
{
    if (m_spriteSheet.isNull())
        return;
    int frameW = 32, frameH = 32;
    QRect src;
    // Faza 0: piąty sprite z pierwszego rzędu (pełne koło)
    // Faza 1: pierwszy sprite z pierwszego rzędu (delikatnie otwarta buzia)
    // Faza 2: pierwszy sprite z drugiego rzędu (mocno otwarta buzia)
    if (m_pacmanFrame % 3 == 0)
    {
        src = QRect(4 * frameW, 0, frameW, frameH); // kolumna 4, rząd 0
    }
    else if (m_pacmanFrame % 3 == 1)
    {
        src = QRect(0, 0, frameW, frameH); // kolumna 0, rząd 0
    }
    else
    {
        src = QRect(0, 1 * frameH, frameW, frameH); // kolumna 0, rząd 1
    }
    QImage frameImg = m_spriteSheet.copy(src).toImage().convertToFormat(QImage::Format_ARGB32);
    // Zamień różowe tło (255,0,255) na przezroczystość
    for (int yPix = 0; yPix < frameImg.height(); ++yPix)
    {
        QRgb *line = reinterpret_cast<QRgb *>(frameImg.scanLine(yPix));
        for (int xPix = 0; xPix < frameImg.width(); ++xPix)
        {
            if (qRed(line[xPix]) == 255 && qGreen(line[xPix]) == 0 && qBlue(line[xPix]) == 255)
            {
                line[xPix] = qRgba(255, 0, 255, 0); // przezroczystość
            }
        }
    }
    p.drawImage(QRect(x, y, size, size), frameImg);
}

// Helper: Draw ghost using sprite sheet
void PacmanOverlay::drawGhost(QPainter &p, int x, int y, int size)
{
    if (m_spriteSheet.isNull())
        return;
    int frameW = 32, frameH = 32;
    // Piąty rząd (indeks 4), pierwszy sprite (indeks 0) – czerwony duch z oczami w lewo
    QRect src(0, 4 * frameH, frameW, frameH);
    QImage frameImg = m_spriteSheet.copy(src).toImage().convertToFormat(QImage::Format_ARGB32);
    // Zamiana różowego tła (255,0,255) na przezroczystość
    for (int yPix = 0; yPix < frameImg.height(); ++yPix)
    {
        QRgb *line = reinterpret_cast<QRgb *>(frameImg.scanLine(yPix));
        for (int xPix = 0; xPix < frameImg.width(); ++xPix)
        {
            if (qRed(line[xPix]) == 255 && qGreen(line[xPix]) == 0 && qBlue(line[xPix]) == 255)
            {
                line[xPix] = qRgba(255, 0, 255, 0);
            }
        }
    }
    p.drawImage(QRect(x, y, size, size), frameImg);
}

// Helper: Draw Pac-Man vanishing animation
void PacmanOverlay::drawPacmanVanish(QPainter &p, int x, int y, int size)
{
    if (m_spriteSheet.isNull())
        return;
    int frame = m_vanishFrame;
    int frameY = 2; // row 2: vanishing
    int frameW = 32, frameH = 32;
    QRect src(frame * frameW, frameY * frameH, frameW, frameH);
    p.drawPixmap(x, y, size, size, m_spriteSheet, src.x(), src.y(), src.width(), src.height());
}

void PacmanOverlay::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_timerId)
    {
        if (!m_showGhost && !m_pacmanVanishing)
        {
            m_pacmanFrame = (m_pacmanFrame + 1) % 3;
            m_pacmanX += s_pacmanSpeedPx;
            update();
        }
        // Ghost nie goni Pac-Mana, tylko stoi
        if (m_showGhost && !m_pacmanVanishing)
        {
            m_ghostFrame = 0; // tylko jeden sprite ducha
            // Duch stoi w miejscu, nic nie zmieniamy
            update();
        }
        // Pac-Man vanishing animation
        if (m_pacmanVanishing)
        {
            m_vanishFrame++;
            if (m_vanishFrame > 5)
            { // Assume 6 vanish frames
                m_pacmanVanishing = false;
            }
            update();
        }
    }
    if (event->timerId() == m_eatCharTimerId)
    {
        if (m_targetWidget && !m_showGhost && !m_pacmanVanishing)
        {
            bool any = false;
            if (auto le = qobject_cast<QLineEdit *>(m_targetWidget))
            {
                QString t = le->text();
                if (!t.isEmpty())
                {
                    le->setText(t.left(t.length() - 1));
                    any = true;
                }
            }
            else if (auto te = qobject_cast<QTextEdit *>(m_targetWidget))
            {
                QString t = te->toPlainText();
                if (!t.isEmpty())
                {
                    te->setPlainText(t.left(t.length() - 1));
                    any = true;
                }
            }
            if (!any)
            {
                killTimer(m_eatCharTimerId);
            }
        }
    }
}
