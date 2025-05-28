#include "PacmanOverlay.h"
#include <QPainter>
#include <QLineEdit>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QTimerEvent>
#include <QPainterPath>
#include <QImage>
#include <QDate>
#include <QtGlobal>      // dla QT_VERSION
#include <QRandomGenerator> // dla QRandomGenerator w Qt 6.x

PacmanOverlay::PacmanOverlay(QWidget *parent)
    : QWidget(parent), m_targetWidget(nullptr), m_angle(0), m_timerId(0), m_eatCharTimerId(0), m_durationMs(5000), m_showGhost(false),
      m_pacmanFrame(0), m_ghostFrame(0), m_vanishFrame(0), m_pacmanVanishing(false), m_collisionHideMs(3000)
{
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
    setVisible(false);  // Domyślnie overlay jest ukryty

    // Ładuj nowy sprite sheet
    m_spriteSheet = QPixmap(":/images/spritesheet.png");
    if (m_spriteSheet.isNull())
    {
        qWarning() << "[PACMAN] Sprite sheet (spritesheet.png) not loaded!";
    }
    // Usunięto connect do m_lifetimeTimer (nie będzie automatycznego znikania)

    // --- Dodane zmienne do animacji położenia Pac-Mana i ducha ---
    m_pacmanX = 0;
    m_ghostX = 0;
    m_ghostChasing = false;
    m_pacmanCollided = false;
    m_collisionFrame = 0;
    m_holdingLastFrame = false;
    m_lastFrameHoldTimeMs = 2000; // 2 sekundy na zatrzymanie ostatniej klatki
}

// --- Statyczne zmienne do regulacji prędkości ---
double PacmanOverlay::s_pacmanSpeedPx = 0.75; // domyślnie 0.75 px na klatkę
int PacmanOverlay::s_eatCharIntervalMs = 100; // domyślnie 100 ms na znak

// --- Statyczne zmienne do obsługi specjalnych dat aktywacji ---
bool PacmanOverlay::s_enableBirthdayActivation = true;         // Domyślnie włączone
int PacmanOverlay::s_birthdayDay = 11;                         // 11 grudnia (urodziny)
int PacmanOverlay::s_birthdayMonth = 12;                       // Grudzień (12)
bool PacmanOverlay::s_enablePacManReleaseActivation = true;    // Domyślnie włączone
bool PacmanOverlay::s_enableRandomActivation = true;           // Domyślnie włączone
int PacmanOverlay::s_randomActivationChance = 10;              // 10% szans na losową aktywację
bool PacmanOverlay::s_enableCapacityActivation = true;         // Domyślnie włączone
int PacmanOverlay::s_capacityCharCount = 22;                   // Domyślnie 22 znaki (premiera Pac-Mana 22.05.1980)

void PacmanOverlay::setTargetWidget(QWidget *target)
{
    qDebug() << "[PACMAN] setTargetWidget: target=" << target << ", parent before=" << parent();
    m_targetWidget = target;
    if (target)
    {
        setParent(target);
        resize(target->size());
        move(0, 0);
        // NIE pokazuj overlay'a automatycznie - zostanie pokazany tylko po wywołaniu start() i tylko gdy warunki aktywacji są spełnione
        setVisible(false);
        qDebug() << "[PACMAN] setTargetWidget: parent after=" << parent() << ", geometry=" << geometry();
    }
}

void PacmanOverlay::start(int durationMs)
{
    qDebug() << "[PACMAN DEBUG] start() called, m_durationMs=" << durationMs << ", m_targetWidget=" << m_targetWidget;
    if (!m_targetWidget)
        return;
        
    // Sprawdź czy nie została wywołana aktywacja z mainwindow.cpp dla tekstu o długości 22 znaków
    QString currentText;
    if (auto le = qobject_cast<QLineEdit*>(m_targetWidget)) {
        currentText = le->text();
    } else if (auto te = qobject_cast<QTextEdit*>(m_targetWidget)) {
        currentText = te->toPlainText();
    } else if (auto pte = qobject_cast<QPlainTextEdit*>(m_targetWidget)) {
        currentText = pte->toPlainText();
    }
    bool capacityActivation = hasCapacityActivation(currentText);
    
    // Jeśli została wywołana bezpośrednio dla tekstu o długości 22 znaki (z mainwindow.cpp),
    // wyświetl odpowiedni komunikat i przejdź do pełnej aktywacji
    if (capacityActivation) {
        qDebug() << "[PACMAN] Aktywacja poprzez dokładną liczbę znaków (" << s_capacityCharCount << ")!";
    } else {
        // Standardowa aktywacja w oparciu o datę lub losową szansę
        bool specialDateActive = isSpecialDate();
        
        // Losowa szansa na aktywację (domyślnie 10%)
        bool randomChance = false;
        if (s_enableRandomActivation) {
            // W Qt 6.x użyj QRandomGenerator, w starszych wersjach qrand()
    #if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
            randomChance = (QRandomGenerator::global()->bounded(100) < s_randomActivationChance);
    #else
            randomChance = (qrand() % 100 < s_randomActivationChance);
    #endif
        }
        
        // Aktywuj Easter Egg tylko w specjalne daty lub losowo (bez wyzwalacza pojemnościowego)
        if (!specialDateActive && !randomChance) {
            qDebug() << "[PACMAN] Easter egg nie aktywowany - nie jest specjalna data, nie wylosowano aktywacji";
            // Upewnij się, że overlay jest ukryty
            setVisible(false);
            // Zatrzymaj animację jeśli była aktywna
            if (m_timerId) {
                killTimer(m_timerId);
                m_timerId = 0;
            }
            if (m_eatCharTimerId) {
                killTimer(m_eatCharTimerId);
                m_eatCharTimerId = 0;
            }
            return;
        }
        
        if (specialDateActive) {
            qDebug() << "[PACMAN] Aktywacja ze względu na specjalną datę!";
        } else if (randomChance) {
            qDebug() << "[PACMAN] Aktywacja losowa!";
        }
    }
    
    // Emituj sygnał, że easter egg został aktywowany
    emit finished();
    
    m_durationMs = durationMs;
    m_showGhost = false;
    // Opóźnij start animacji o 5 sekund
    setVisible(false); // Ukryj overlay do czasu startu
    QTimer::singleShot(5000, this, [this]()
                       {
        setVisible(true);
        m_angle = 0;
        m_mouthDelta = 0;
        m_mouthClosing = false;
        m_timerId = startTimer(40);                         // timer do animacji szczęki
        m_eatCharTimerId = startTimer(s_eatCharIntervalMs); // timer do zjadania znaków (regulowany)
        m_pacmanX = 0;
        m_ghostChasing = false;
        m_ghostX = 0;
        if (auto le = qobject_cast<QLineEdit *>(m_targetWidget))
        {
            m_initialText = le->text();
        }
        else if (auto te = qobject_cast<QTextEdit *>(m_targetWidget))
        {
            m_initialText = te->toPlainText();
        }
        else if (auto pte = qobject_cast<QPlainTextEdit *>(m_targetWidget))
        {
            m_initialText = pte->toPlainText();
        }
        else
        {
            m_initialText.clear();
        }
        qDebug() << "[PACMAN DEBUG] initialText=" << m_initialText;
        // Wyliczanie parametrów ruchu ducha
        int numChars = m_initialText.length();
        int eatCharInterval = s_eatCharIntervalMs; // ms na znak
        int totalEatTimeMs = numChars * eatCharInterval;
        int ghostDelayMs = 500; // opóźnienie startu ducha
        int frameIntervalMs = 40; // timerId = 40 ms
        int effectiveChaseTimeMs = totalEatTimeMs - ghostDelayMs;
        int numFrames = (effectiveChaseTimeMs > 0) ? effectiveChaseTimeMs / frameIntervalMs : 1;
        
        // Obliczanie szerokości tekstu i pojedynczego znaku
        int textWidth = 0;
        int charWidth = 0;
        if (auto le = qobject_cast<QLineEdit *>(m_targetWidget))
        {
            QFontMetrics fm(le->font());
            textWidth = fm.horizontalAdvance(le->text());
            charWidth = le->text().isEmpty() ? fm.horizontalAdvance(QLatin1Char(' ')) : fm.horizontalAdvance(le->text().at(0));
        }
        else if (auto te = qobject_cast<QTextEdit *>(m_targetWidget))
        {
            QFontMetrics fm(te->font());
            textWidth = fm.horizontalAdvance(te->toPlainText());
            QString txt = te->toPlainText();
            charWidth = txt.isEmpty() ? fm.horizontalAdvance(QLatin1Char(' ')) : fm.horizontalAdvance(txt.at(0));
        }
        else if (auto pte = qobject_cast<QPlainTextEdit *>(m_targetWidget))
        {
            QFontMetrics fm(pte->font());
            textWidth = fm.horizontalAdvance(pte->toPlainText());
            QString txt = pte->toPlainText();
            charWidth = txt.isEmpty() ? fm.horizontalAdvance(QLatin1Char(' ')) : fm.horizontalAdvance(txt.at(0));
        }
        
        // Pacman pojawia się zaraz za tekstem
        int pacmanStartX = textWidth;
        // Duch pojawia się w odległości równej długości tekstu od końca tekstu
        int emptySpaceWidthInChars = numChars; // Odległość w znakach równa liczbie znaków w tekście
        int ghostStartX = textWidth + (emptySpaceWidthInChars * charWidth);
        
        int pacmanEndX = 0;
        int ghostEndX = pacmanEndX; // Duch ma dopaść Pac-Mana przy lewej krawędzi
        int ghostDistance = ghostStartX - ghostEndX;
        m_ghostSpeedPx = (numFrames > 0) ? (double)ghostDistance / numFrames : ghostDistance; // teleport jeśli za późno
        m_ghostX = ghostStartX;
        qDebug() << "[PACMAN DEBUG] ghostDelayMs=" << ghostDelayMs << ", totalEatTimeMs=" << totalEatTimeMs << ", m_ghostSpeedPx=" << m_ghostSpeedPx;
        qDebug() << "[PACMAN DEBUG] numChars=" << numChars << ", textWidth=" << textWidth << ", charWidth=" << charWidth 
                 << ", ghostStartX=" << ghostStartX << ", pacmanStartX=" << pacmanStartX;
        // Duch pojawia się po 1.5 sekundy na prawej krawędzi pola tekstowego i zaczyna gonić Pac-Mana
        QTimer::singleShot(ghostDelayMs, this, [this]() {
            m_showGhost = true;
            m_ghostChasing = true;
            update();
            qDebug() << "[PACMAN DEBUG] Ghost chase started!";
        }); });
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
    else if (auto pte = qobject_cast<QPlainTextEdit *>(m_targetWidget))
    {
        QFontMetrics fm(pte->font());
        textWidth = fm.horizontalAdvance(pte->toPlainText());
    }
    int startX = textWidth;
    // Pac-Man nie znika, zatrzymuje się przy lewej krawędzi
    int pacmanX = std::max(startX - m_pacmanX, 0);
    drawPacman(p, pacmanX, y, size);
    if (m_showGhost)
    {
        drawGhost(p, (int)m_ghostX, y, size);
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
    if (m_pacmanCollided)
    {
        // Najpierw klatki 0-6 z rzędu 6 (ostatni rząd), potem 0-10 z rzędu 6 (ostatni rząd)
        if (m_collisionFrame < 7)
        {
            src = QRect(m_collisionFrame * frameW, 6 * frameH, frameW, frameH);
        }
        else
        {
            int frameInRow = (m_collisionFrame - 7) % 11;
            src = QRect(frameInRow * frameW, 6 * frameH, frameW, frameH);
        }
    }
    else
    {
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
        if (!m_pacmanVanishing)
        {
            if (m_pacmanCollided)
            {
                // Jeśli trzymamy ostatnią klatkę, nie rób nic
                if (!m_holdingLastFrame)
                {
                    // Animacja kolizji: 0-6 z rzędu 6, potem 0-10 z rzędu 6
                    if (m_collisionFrame < 7 + 11 - 1)
                    {
                        m_collisionFrame++;
                        
                        // Sprawdź, czy osiągnęliśmy ostatnią klatkę (7+10=17)
                        if (m_collisionFrame == 7 + 11 - 1)
                        {
                            m_holdingLastFrame = true;
                            qDebug() << "[PACMAN] Zatrzymuję ostatnią klatkę animacji na" << m_lastFrameHoldTimeMs << "ms";
                            
                            // Ustaw timer, po którym ukryjemy overlay
                            QTimer::singleShot(m_lastFrameHoldTimeMs, this, [this]()
                            {
                                this->setVisible(false);
                                // Resetuj stan animacji
                                m_pacmanCollided = false;
                                m_collisionFrame = 0;
                                m_pacmanX = 0;
                                m_ghostX = 0;
                                m_ghostChasing = false;
                                m_showGhost = false;
                                m_holdingLastFrame = false;
                                
                                // Emituj sygnał, że animacja się zakończyła (na wszelki wypadek, choć już emitowaliśmy)
                                emit finished();
                            });
                        }
                    }
                }
            }
            else
            {
                m_pacmanFrame = (m_pacmanFrame + 1) % 3;
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
                else if (auto pte = qobject_cast<QPlainTextEdit *>(m_targetWidget))
                {
                    QFontMetrics fm(pte->font());
                    textWidth = fm.horizontalAdvance(pte->toPlainText());
                }
                int nextX = textWidth - (m_pacmanX + s_pacmanSpeedPx);
                if (nextX >= 0)
                    m_pacmanX += s_pacmanSpeedPx;
            }
            update();
        }
        // Duch goni Pac-Mana
        if (m_ghostChasing)
        {
            // Oblicz aktualną pozycję Pac-Mana
            int size = 32; // rozmiar Pac-Mana i ducha
            int pacmanX = 0;
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
            else if (auto pte = qobject_cast<QPlainTextEdit *>(m_targetWidget))
            {
                QFontMetrics fm(pte->font());
                textWidth = fm.horizontalAdvance(pte->toPlainText());
            }

            // Oblicz pozycje Pac-Mana i ducha
            pacmanX = std::max(textWidth - m_pacmanX, 0);
            double nextGhostX = m_ghostX - m_ghostSpeedPx;

            // Sprawdź czy nastąpi kolizja (duch dotyka Pac-Mana)
            if (nextGhostX <= pacmanX + size)
            {
                // Zatrzymaj ducha tuż przed Pac-Manem, nie nachodzą na siebie
                m_ghostX = pacmanX + size; // Pozycja ducha dokładnie przy prawej krawędzi Pac-Mana
                m_ghostChasing = false;    // Zatrzymaj ruch ducha

                // Zatrzymaj także timer zjadania znaków
                if (m_eatCharTimerId)
                {
                    killTimer(m_eatCharTimerId);
                    m_eatCharTimerId = 0;
                }

                // Aktywuj animację kolizji dla Pac-Mana (klatki z 7. rzędu)
                m_pacmanCollided = true;
                m_collisionFrame = 0;
                m_holdingLastFrame = false;
                qDebug() << "[PACMAN] Duch dogonił Pac-Mana! Rozpoczynam animację kolizji.";
                update();
            }
            else
            {
                // Kontynuuj ruch ducha
                m_ghostX = nextGhostX;
                if (m_ghostX < 0)
                    m_ghostX = 0;
            }
            update();
        }
        // Pac-Man vanishing animation (nieużywane w tej wersji)
        if (m_pacmanVanishing)
        {
            m_vanishFrame++;
            if (m_vanishFrame > 5)
            {
                m_pacmanVanishing = false;
            }
            update();
        }
    }
    if (event->timerId() == m_eatCharTimerId)
    {
        if (m_targetWidget && !m_pacmanVanishing)
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
            else if (auto pte = qobject_cast<QPlainTextEdit *>(m_targetWidget))
            {
                QString t = pte->toPlainText();
                if (!t.isEmpty())
                {
                    pte->setPlainText(t.left(t.length() - 1));
                    any = true;
                }
            }
            // Jeśli nie ma już znaków, Pac-Man się zatrzymuje, ale nie znika
            if (!any)
            {
                killTimer(m_eatCharTimerId);
                m_eatCharTimerId = 0;
                // Nie zatrzymujemy ducha tutaj - duch będzie zatrzymany dopiero przy kolizji z Pac-Manem
            }
        }
    }
}

// Helper: Sprawdza czy obecna data jest jedną ze specjalnych dat (urodziny lub premiera Pac-Mana)
bool PacmanOverlay::isSpecialDate() const
{
    QDate currentDate = QDate::currentDate();
    int currentDay = currentDate.day();
    int currentMonth = currentDate.month();
    
    // Sprawdź czy to dzień premiery Pac-Mana (22 maja 1980)
    bool isPacManReleaseDate = (currentDay == 22 && currentMonth == 5 && s_enablePacManReleaseActivation);
    
    // Sprawdź czy to urodziny użytkownika (11 grudnia)
    bool isBirthday = (currentDay == s_birthdayDay && currentMonth == s_birthdayMonth && s_enableBirthdayActivation);
    
    // Zwróć true, jeśli to którakolwiek ze specjalnych dat
    return isPacManReleaseDate || isBirthday;
}

// Helper: Sprawdza czy tekst ma dokładnie wymaganą liczbę znaków (domyślnie 22)
bool PacmanOverlay::hasCapacityActivation(const QString &text) const
{
    if (!s_enableCapacityActivation)
        return false;
        
    return text.length() == s_capacityCharCount;
}
