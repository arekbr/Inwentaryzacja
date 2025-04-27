/**
 * @file photoitem.h
 * @brief Deklaracja klasy PhotoItem reprezentującej interaktywny element graficzny zdjęcia.
 * @author Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK
 * @author ChatGPT
 * @author GROK
 * @version 1.1.8
 * @date 2025-04-25
 *
 * Plik zawiera deklarację klasy PhotoItem, która rozszerza funkcjonalność QGraphicsPixmapItem
 * o obsługę interakcji użytkownika (kliknięcia, najechanie kursorem, opuszczenie kursora)
 * oraz możliwość wizualnego zaznaczenia elementu za pomocą ramki.
 *
 * Klasa PhotoItem emitując sygnały, umożliwia integrację z logiką aplikacji,
 * np. wybieranie zdjęcia, podświetlanie lub reakcję na kliknięcie.
 */

#ifndef PHOTOITEM_H
#define PHOTOITEM_H

#include <QGraphicsPixmapItem>
#include <QObject>
#include <QGraphicsRectItem>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>

/**
 * @class PhotoItem
 * @brief Element graficzny reprezentujący zdjęcie z obsługą zaznaczenia i zdarzeń interaktywnych.
 *
 * Klasa PhotoItem dziedziczy po QGraphicsPixmapItem i QObject, co umożliwia
 * wyświetlanie grafiki na scenie graficznej Qt oraz emitowanie sygnałów Qt.
 *
 * Umożliwia reagowanie na kliknięcia i zdarzenia kursora myszy (hover),
 * wizualne oznaczenie zaznaczenia oraz emitowanie sygnałów do logiki aplikacji.
 */
class PhotoItem : public QObject, public QGraphicsPixmapItem {
    Q_OBJECT

public:
    /**
     * @brief Konstruktor klasy PhotoItem.
     * @param parent Opcjonalny wskaźnik na element nadrzędny QGraphicsItem.
     *
     * Inicjalizuje obiekt, ustawia domyślne właściwości oraz umożliwia reagowanie
     * na zdarzenia kursora myszy.
     */
    explicit PhotoItem(QGraphicsItem *parent = nullptr);

    /**
     * @brief Destruktor klasy PhotoItem.
     *
     * Usuwa dynamicznie alokowaną ramkę zaznaczenia (jeśli istnieje).
     */
    ~PhotoItem();

    /**
     * @brief Ustawia stan zaznaczenia obiektu.
     * @param selected Wartość true zaznacza element, false odznacza.
     *
     * Zmiana stanu powoduje aktualizację ramki graficznej (m_frame).
     */
    void setSelected(bool selected);

signals:
    /**
     * @brief Sygnał emitowany po jednokrotnym kliknięciu lewym przyciskiem myszy.
     * @param item Wskaźnik na kliknięty element PhotoItem.
     *
     * Używany do otwierania podglądu zdjęcia w naturalnym rozmiarze.
     */
    void clicked(PhotoItem *item);

    /**
     * @brief Sygnał emitowany po dwukrotnym kliknięciu lewym przyciskiem myszy.
     * @param item Wskaźnik na kliknięty element PhotoItem.
     *
     * Używany do otwierania zdjęcia w trybie pełnoekranowym z możliwością powiększania i przesuwania.
     */
    void doubleClicked(PhotoItem *item);

    /**
     * @brief Sygnał emitowany po najechaniu kursorem na obiekt.
     * @param item Wskaźnik do obiektu PhotoItem, na który najechano.
     */
    void hovered(PhotoItem *item);

    /**
     * @brief Sygnał emitowany po opuszczeniu kursora z obiektu.
     * @param item Wskaźnik do obiektu PhotoItem, który przestał być hoverowany.
     */
    void unhovered(PhotoItem *item);

protected:
    /**
     * @brief Obsługa zdarzenia kliknięcia myszy.
     * @param event Obiekt zawierający dane o zdarzeniu myszy.
     *
     * Przechwytuje kliknięcia i emituje sygnał clicked().
     */
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

    /**
     * @brief Obsługa zdarzenia podwójnego kliknięcia myszy.
     * @param event Obiekt zawierający dane o zdarzeniu podwójnego kliknięcia.
     *
     * Przechwytuje podwójne kliknięcia i emituje sygnał doubleClicked().
     */
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

    /**
     * @brief Obsługa zdarzenia najechania kursora.
     * @param event Obiekt zawierający dane o zdarzeniu hover enter.
     *
     * Emisja sygnału hovered() po najechaniu na obiekt.
     */
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;

    /**
     * @brief Obsługa zdarzenia opuszczenia kursora.
     * @param event Obiekt zawierający dane o zdarzeniu hover leave.
     *
     * Emisja sygnału unhovered() po opuszczeniu obiektu przez kursor.
     */
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

private:
    /**
     * @brief Flaga oznaczająca, czy obiekt został kliknięty.
     */
    bool m_pressed;

    /**
     * @brief Flaga oznaczająca, czy obiekt jest zaznaczony.
     */
    bool m_selected;

    /**
     * @brief Ramka graficzna wykorzystywana do oznaczania zaznaczenia.
     */
    QGraphicsRectItem *m_frame;

    /**
     * @brief Aktualizuje wygląd ramki zaznaczenia w zależności od stanu.
     */
    void updateFrame();
};

#endif // PHOTOITEM_H
