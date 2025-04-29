/**
 * @file types.h
 * @brief Deklaracja klasy types służącej do zarządzania typami eksponatów w aplikacji inwentaryzacyjnej.
 * @version 1.1.8
 * @date 2025-04-25
 * @author
 * - Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK
 * - ChatGPT
 * - GROK
 *
 * Plik zawiera deklarację klasy `types`, reprezentującej okno dialogowe
 * umożliwiające użytkownikowi zarządzanie typami eksponatów (np. Komputer, Monitor, Drukarka).
 * Klasa współpracuje z `MainWindow`, umożliwiając odświeżenie odpowiedniego pola combo box
 * po zakończeniu edycji typów.
 *
 * Operacje są wykonywane na tabeli `types` w bazie danych.
 */

#ifndef TYPES_H
#define TYPES_H

#include <QDialog>
#include <QSqlDatabase>

class MainWindow;

namespace Ui {
class types;
}

/**
 * @class types
 * @brief Okno dialogowe do zarządzania typami eksponatów.
 *
 * Klasa dziedziczy po QDialog i udostępnia interfejs do dodawania, edytowania
 * i usuwania typów sprzętu w bazie danych. Zmiany są synchronizowane z głównym oknem aplikacji.
 */
class types : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Konstruktor klasy types.
     * @param parent Wskaźnik na nadrzędny widget (domyślnie nullptr).
     *
     * Inicjalizuje interfejs użytkownika oraz połączenie z bazą danych.
     */
    explicit types(QWidget *parent = nullptr);

    /**
     * @brief Destruktor klasy types.
     *
     * Usuwa interfejs użytkownika i zwalnia zasoby.
     */
    ~types();

    /**
     * @brief Ustawia wskaźnik na główne okno aplikacji.
     * @param mainWindow Wskaźnik do obiektu klasy MainWindow.
     *
     * Pozwala na synchronizację danych w głównym widoku aplikacji.
     */
    void setMainWindow(MainWindow *mainWindow);

private slots:
    /**
     * @brief Slot obsługujący dodawanie nowego typu.
     *
     * Wywoływany po kliknięciu przycisku „Dodaj”.
     */
    void onAddClicked();

    /**
     * @brief Slot obsługujący edytowanie zaznaczonego typu.
     *
     * Wywoływany po kliknięciu przycisku „Edytuj”.
     */
    void onEditClicked();

    /**
     * @brief Slot obsługujący usunięcie zaznaczonego typu.
     *
     * Wywoływany po kliknięciu przycisku „Usuń”.
     */
    void onDeleteClicked();

    /**
     * @brief Slot obsługujący zatwierdzenie zmian i zamknięcie okna.
     *
     * Wywoływany po kliknięciu przycisku „OK”. Odświeża pole wyboru typów w głównym oknie.
     */
    void onOkClicked();

private:
    /**
     * @brief Odświeża listę typów eksponatów w interfejsie.
     *
     * Pobiera dane z bazy i ustawia jako model dla widoku listy.
     */
    void refreshList();

    /// Wskaźnik do interfejsu użytkownika wygenerowanego przez Qt Designer.
    Ui::types *ui;

    /// Wskaźnik na główne okno aplikacji, wykorzystywany do odświeżenia pól wyboru.
    MainWindow *m_mainWindow;

    /// Obiekt połączenia z bazą danych.
    QSqlDatabase m_db;
};

#endif // TYPES_H
