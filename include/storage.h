/**
 * @file storage.h
 * @brief Deklaracja klasy storage służącej do zarządzania miejscami przechowywania eksponatów w aplikacji inwentaryzacyjnej.
 * @version \projectnumber
 * @date 2025-05-03
 * @author Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK & ChatGPT & GROK
 *
 * @section Overview
 * Plik zawiera deklarację klasy storage, która reprezentuje okno dialogowe do zarządzania
 * miejscami przechowywania eksponatów (np. magazyny, półki, pokoje) w aplikacji inwentaryzacyjnej.
 * Klasa umożliwia dodawanie, edytowanie i usuwanie lokalizacji w tabeli `storage_places` w bazie danych,
 * a także synchronizację z głównym oknem aplikacji (`MainWindow`) w celu odświeżenia combo boxów
 * po zapisaniu zmian.
 *
 * @section Structure
 * Plik nagłówkowy zawiera:
 * 1. **Deklarację klasy storage** – dziedziczy po QDialog.
 * 2. **Metody publiczne** – konstruktor, destruktor, ustawianie MainWindow.
 * 3. **Sloty prywatne** – obsługują akcje użytkownika (dodawanie, edycja, usuwanie, zatwierdzanie).
 * 4. **Metody prywatne** – odświeżanie listy lokalizacji.
 * 5. **Zmienne prywatne** – przechowują interfejs, MainWindow i połączenie z bazą danych.
 *
 * @section Dependencies
 * - **Qt Framework**: Używa klas QDialog, QSqlDatabase.
 * - **Nagłówki aplikacji**: mainwindow.h (deklaracja MainWindow).
 * - **Namespace Ui**: Zawiera definicję interfejsu użytkownika (ui_storage.h).
 *
 * @section Notes
 * - Kod nie został zmodyfikowany, zgodnie z wymaganiami użytkownika. Dodano jedynie komentarze i dokumentację.
 * - Klasa jest częścią systemu słownikowego, współpracuje z MainWindow do aktualizacji combo boxów.
 * - Obsługuje MySQL, ale aplikacja wspiera także SQLite (konfigurowane w DatabaseConfigDialog).
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
 * @section ClassOverview
 * Klasa storage dziedziczy po QDialog i zapewnia interfejs do dodawania, edytowania
 * i usuwania miejsc przechowywania eksponatów (np. regały, magazyny) w bazie danych
 * (tabela `storage_places`). Współpracuje z klasą MainWindow, aby odświeżyć listę
 * lokalizacji w combo boxie po zapisaniu zmian.
 *
 * @section Responsibilities
 * - Wyświetlanie listy lokalizacji w QListView.
 * - Dodawanie nowych lokalizacji do tabeli `storage_places` z unikalnym UUID.
 * - Edycja istniejących lokalizacji (zmiana nazwy).
 * - Usuwanie lokalizacji po potwierdzeniu użytkownika.
 * - Odświeżanie combo boxa lokalizacji w MainWindow po zapisaniu zmian.
 */
class storage : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Konstruktor klasy storage.
     * @param parent Wskaźnik na nadrzędny widget (domyślnie nullptr).
     *
     * @section ConstructorOverview
     * Inicjalizuje okno dialogowe, konfiguruje interfejs użytkownika,
     * ustanawia połączenie z bazą danych i podłącza sloty dla przycisków.
     */
    explicit storage(QWidget *parent = nullptr);

    /**
     * @brief Destruktor klasy storage.
     *
     * @section DestructorOverview
     * Zwalnia zasoby interfejsu użytkownika i usuwa obiekty dynamiczne.
     */
    ~storage();

    /**
     * @brief Ustawia wskaźnik na główne okno aplikacji.
     * @param mainWindow Wskaźnik na obiekt klasy MainWindow.
     *
     * @section MethodOverview
     * Umożliwia komunikację z głównym oknem w celu odświeżenia listy lokalizacji
     * w combo boxie po zapisaniu zmian.
     */
    void setMainWindow(MainWindow *mainWindow);

private slots:
    /**
     * @brief Slot wywoływany po kliknięciu przycisku „Dodaj”.
     *
     * @section SlotOverview
     * Pobiera nazwę lokalizacji z pola tekstowego, generuje UUID, zapisuje lokalizację
     * w tabeli `storage_places` i odświeża listę.
     */
    void onAddClicked();

    /**
     * @brief Slot wywoływany po kliknięciu przycisku „Edytuj”.
     *
     * @section SlotOverview
     * Pobiera wybraną lokalizację, otwiera okno dialogowe do edycji nazwy,
     * aktualizuje rekord w tabeli `storage_places` i odświeża listę.
     */
    void onEditClicked();

    /**
     * @brief Slot wywoływany po kliknięciu przycisku „Usuń”.
     *
     * @section SlotOverview
     * Usuwa wybraną lokalizację z tabeli `storage_places` po potwierdzeniu użytkownika
     * i odświeża listę.
     */
    void onDeleteClicked();

    /**
     * @brief Slot wywoływany po kliknięciu przycisku OK.
     *
     * @section SlotOverview
     * Odświeża listę lokalizacji w combo boxie MainWindow (jeśli ustawione),
     * zamyka okno dialogowe z wynikiem zaakceptowania.
     */
    void onOkClicked();

private:
    /**
     * @brief Odświeża listę lokalizacji w widoku listy.
     *
     * @section MethodOverview
     * Pobiera lokalizacje z tabeli `storage_places`, sortuje alfabetycznie
     * i wyświetla w QListView za pomocą QSqlQueryModel.
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
