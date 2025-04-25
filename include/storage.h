/**
 * @file storage.h
 * @brief Deklaracja klasy storage służącej do zarządzania miejscami przechowywania eksponatów w aplikacji inwentaryzacyjnej.
 * @version 1.1.8
 * @date 2025-04-25
 * @author
 * - Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK
 * - ChatGPT
 * - GROK
 *
 * Plik zawiera deklarację klasy `storage`, reprezentującej okno dialogowe
 * umożliwiające dodawanie, edytowanie i usuwanie lokalizacji (np. magazynów, półek, pokoi)
 * wykorzystywanych jako miejsca przechowywania eksponatów.
 *
 * Klasa współpracuje z `MainWindow`, by po zatwierdzeniu zmian możliwe było
 * odświeżenie odpowiedniego pola typu combo box w głównym widoku aplikacji.
 * Operacje wykonywane są na tabeli `storages` w bazie danych.
 */

#ifndef STORAGE_H
#define STORAGE_H

#include <QDialog>
#include <QSqlDatabase>

class MainWindow;

namespace Ui {
class storage;
}

/**
 * @class storage
 * @brief Okno dialogowe do zarządzania miejscami przechowywania eksponatów.
 *
 * Klasa dziedziczy po QDialog i zapewnia interfejs do edycji danych lokalizacyjnych
 * eksponatów (np. regały, szafy, magazyny). Użytkownik może dodawać, edytować i usuwać
 * wpisy z bazy danych. Synchronizacja z głównym oknem umożliwia automatyczne
 * odświeżenie pól wyboru lokalizacji.
 */
class storage : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Konstruktor klasy storage.
     * @param parent Wskaźnik na nadrzędny widget (domyślnie nullptr).
     *
     * Inicjalizuje interfejs graficzny oraz ustawia połączenie z bazą danych.
     */
    explicit storage(QWidget *parent = nullptr);

    /**
     * @brief Destruktor klasy storage.
     *
     * Zwalnia zasoby interfejsu użytkownika.
     */
    ~storage();

    /**
     * @brief Ustawia wskaźnik na główne okno aplikacji.
     * @param mainWindow Wskaźnik na obiekt klasy MainWindow.
     *
     * Pozwala na synchronizację pól w głównym oknie po zatwierdzeniu zmian.
     */
    void setMainWindow(MainWindow *mainWindow);

private slots:
    /**
     * @brief Slot wywoływany po kliknięciu przycisku „Dodaj”.
     *
     * Dodaje nową lokalizację do bazy danych.
     */
    void onAddClicked();

    /**
     * @brief Slot wywoływany po kliknięciu przycisku „Edytuj”.
     *
     * Edytuje aktualnie zaznaczoną lokalizację.
     */
    void onEditClicked();

    /**
     * @brief Slot wywoływany po kliknięciu przycisku „Usuń”.
     *
     * Usuwa zaznaczoną lokalizację z bazy danych po potwierdzeniu.
     */
    void onDeleteClicked();

    /**
     * @brief Slot wywoływany po kliknięciu przycisku OK.
     *
     * Zamyka okno dialogowe i odświeża combo box z lokalizacjami w głównym oknie.
     */
    void onOkClicked();

private:
    /**
     * @brief Odświeża listę lokalizacji w widoku listy.
     *
     * Pobiera dane z tabeli `storages` w bazie danych i ustawia je w komponencie listy.
     */
    void refreshList();

    /// Wskaźnik na interfejs graficzny okna dialogowego.
    Ui::storage *ui;

    /// Wskaźnik na główne okno aplikacji (do synchronizacji danych).
    MainWindow *m_mainWindow;

    /// Obiekt połączenia z bazą danych.
    QSqlDatabase m_db;
};

#endif // STORAGE_H
