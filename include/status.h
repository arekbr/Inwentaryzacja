/**
 * @file status.h
 * @brief Deklaracja klasy status służącej do zarządzania statusami eksponatów w aplikacji inwentaryzacyjnej.
 * @version 1.1.8
 * @date 2025-04-25
 * @author Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK
 * @author ChatGPT
 * @author GROK
 *
 * Plik zawiera deklarację klasy status, która reprezentuje okno dialogowe
 * umożliwiające użytkownikowi dodawanie, edytowanie i usuwanie statusów eksponatów.
 * Klasa operuje na tabeli `status` w bazie danych i umożliwia synchronizację z głównym oknem aplikacji
 * w celu odświeżenia odpowiednich pól typu combo box po zapisaniu zmian.
 *
 * Klasa współpracuje z `MainWindow` i korzysta z interfejsu użytkownika wygenerowanego przez Qt Designer.
 */

#ifndef STATUS_H
#define STATUS_H

#include <QDialog>
#include <QSqlDatabase>

class MainWindow;

namespace Ui {
class status;
}

/**
 * @class status
 * @brief Okno dialogowe do zarządzania statusami eksponatów.
 *
 * Klasa dziedziczy po QDialog i umożliwia edycję listy statusów eksponatów
 * (np. Nowy, Używany, Uszkodzony itp.) zapisanych w bazie danych.
 * Zmiany dokonane przez użytkownika mogą być natychmiast zsynchronizowane z
 * głównym oknem aplikacji.
 */
class status : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Konstruktor klasy status.
     * @param parent Wskaźnik na nadrzędny widget (domyślnie nullptr).
     *
     * Inicjalizuje interfejs użytkownika, łączy sygnały przycisków ze slotami
     * oraz ustawia połączenie z bazą danych.
     */
    explicit status(QWidget *parent = nullptr);

    /**
     * @brief Destruktor klasy status.
     *
     * Zwalnia zasoby interfejsu użytkownika.
     */
    ~status();

    /**
     * @brief Ustawia wskaźnik na główne okno aplikacji.
     * @param mainWindow Wskaźnik na obiekt MainWindow.
     *
     * Umożliwia synchronizację z głównym oknem, np. odświeżanie pól wyboru statusów.
     */
    void setMainWindow(MainWindow *mainWindow);

private slots:
    /**
     * @brief Slot wywoływany po kliknięciu przycisku „Dodaj”.
     *
     * Dodaje nowy status do bazy danych.
     */
    void onAddClicked();

    /**
     * @brief Slot wywoływany po kliknięciu przycisku „Edytuj”.
     *
     * Edytuje aktualnie zaznaczony status.
     */
    void onEditClicked();

    /**
     * @brief Slot wywoływany po kliknięciu przycisku „Usuń”.
     *
     * Usuwa zaznaczony status z bazy danych po potwierdzeniu.
     */
    void onDeleteClicked();

    /**
     * @brief Slot wywoływany po kliknięciu przycisku OK.
     *
     * Zamyka okno dialogowe i odświeża combo box ze statusami w głównym oknie.
     */
    void onOkClicked();

private:
    /**
     * @brief Odświeża listę statusów wyświetlanych w interfejsie.
     *
     * Pobiera dane z tabeli `status` w bazie danych i uaktualnia widok listy.
     */
    void refreshList();

    /// Wskaźnik na obiekt interfejsu użytkownika.
    Ui::status *ui;

    /// Wskaźnik na główne okno aplikacji.
    MainWindow *m_mainWindow;

    /// Obiekt reprezentujący połączenie z bazą danych.
    QSqlDatabase m_db;
};

#endif // STATUS_H
