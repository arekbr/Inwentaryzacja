/**
 * @file status.h
 * @brief Deklaracja klasy status służącej do zarządzania statusami eksponatów w aplikacji inwentaryzacyjnej.
 * @version 1.2.2
 * @date 2025-05-03
 * @author Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK & ChatGPT & GROK
 *
 * @section Overview
 * Plik zawiera deklarację klasy status, która reprezentuje okno dialogowe do zarządzania
 * statusami eksponatów (np. "Nowy", "Używany", "Uszkodzony") w aplikacji inwentaryzacyjnej.
 * Klasa umożliwia dodawanie, edytowanie i usuwanie statusów w tabeli `statuses` w bazie danych,
 * a także synchronizację z głównym oknem aplikacji (`MainWindow`) w celu odświeżenia
 * combo boxów po zapisaniu zmian.
 *
 * @section Structure
 * Plik nagłówkowy zawiera:
 * 1. **Deklarację klasy status** – dziedziczy po QDialog.
 * 2. **Metody publiczne** – konstruktor, destruktor, ustawianie MainWindow.
 * 3. **Sloty prywatne** – obsługują akcje użytkownika (dodawanie, edycja, usuwanie, zatwierdzanie).
 * 4. **Metody prywatne** – odświeżanie listy statusów.
 * 5. **Zmienne prywatne** – przechowują interfejs, MainWindow i połączenie z bazą danych.
 *
 * @section Dependencies
 * - **Qt Framework**: Używa klas QDialog, QSqlDatabase.
 * - **Nagłówki aplikacji**: mainwindow.h (deklaracja MainWindow).
 * - **Namespace Ui**: Zawiera definicję interfejsu użytkownika (ui_status.h).
 *
 * @section Notes
 * - Kod nie został zmodyfikowany, zgodnie z wymaganiami użytkownika. Dodano jedynie komentarze i dokumentację.
 * - Klasa jest częścią systemu słownikowego, współpracuje z MainWindow do aktualizacji combo boxów.
 * - Obsługuje MySQL, ale aplikacja wspiera także SQLite (konfigurowane w DatabaseConfigDialog).
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
 * @section ClassOverview
 * Klasa status dziedziczy po QDialog i zapewnia interfejs do dodawania, edytowania
 * i usuwania statusów eksponatów w bazie danych (tabela `statuses`). Współpracuje
 * z klasą MainWindow, aby odświeżyć listę statusów w combo boxie po zapisaniu zmian.
 *
 * @section Responsibilities
 * - Wyświetlanie listy statusów w QListView.
 * - Dodawanie nowych statusów do tabeli `statuses` z unikalnym UUID.
 * - Edycja istniejących statusów (zmiana nazwy).
 * - Usuwanie statusów po potwierdzeniu użytkownika.
 * - Odświeżanie combo boxa statusów w MainWindow po zapisaniu zmian.
 */
class status : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Konstruktor klasy status.
     * @param parent Wskaźnik na nadrzędny widget (domyślnie nullptr).
     *
     * @section ConstructorOverview
     * Inicjalizuje okno dialogowe, konfiguruje interfejs użytkownika,
     * ustanawia połączenie z bazą danych i podłącza sloty dla przycisków.
     */
    explicit status(QWidget *parent = nullptr);

    /**
     * @brief Destruktor klasy status.
     *
     * @section DestructorOverview
     * Zwalnia zasoby interfejsu użytkownika i usuwa obiekty dynamiczne.
     */
    ~status();

    /**
     * @brief Ustawia wskaźnik na główne okno aplikacji.
     * @param mainWindow Wskaźnik na obiekt MainWindow.
     *
     * @section MethodOverview
     * Umożliwia komunikację z głównym oknem w celu odświeżenia listy statusów
     * w combo boxie po zapisaniu zmian.
     */
    void setMainWindow(MainWindow *mainWindow);

private slots:
    /**
     * @brief Slot wywoływany po kliknięciu przycisku „Dodaj”.
     *
     * @section SlotOverview
     * Pobiera nazwę statusu z pola tekstowego, generuje UUID, zapisuje status
     * w tabeli `statuses` i odświeża listę.
     */
    void onAddClicked();

    /**
     * @brief Slot wywoływany po kliknięciu przycisku „Edytuj”.
     *
     * @section SlotOverview
     * Pobiera wybrany status, otwiera okno dialogowe do edycji nazwy,
     * aktualizuje rekord w tabeli `statuses` i odświeża listę.
     */
    void onEditClicked();

    /**
     * @brief Slot wywoływany po kliknięciu przycisku „Usuń”.
     *
     * @section SlotOverview
     * Usuwa wybrany status z tabeli `statuses` po potwierdzeniu użytkownika
     * i odświeża listę.
     */
    void onDeleteClicked();

    /**
     * @brief Slot wywoływany po kliknięciu przycisku OK.
     *
     * @section SlotOverview
     * Odświeża listę statusów w combo boxie MainWindow (jeśli ustawione),
     * zamyka okno dialogowe z wynikiem zaakceptowania.
     */
    void onOkClicked();

private:
    /**
     * @brief Odświeża listę statusów wyświetlanych w interfejsie.
     *
     * @section MethodOverview
     * Pobiera statusy z tabeli `statuses`, sortuje alfabetycznie
     * i wyświetla w QListView za pomocą QSqlQueryModel.
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
