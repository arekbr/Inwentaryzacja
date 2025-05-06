/**
 * @file vendors.h
 * @brief Deklaracja klasy vendors do zarządzania producentami sprzętu w aplikacji inwentaryzacyjnej.
 * @version \projectnumber
 * @date 2025-05-03
 * @author Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK & ChatGPT & GROK
 *
 * @section Overview
 * Plik zawiera deklarację klasy vendors, która reprezentuje okno dialogowe do zarządzania
 * producentami sprzętu (np. Commodore, IBM, Apple) w aplikacji inwentaryzacyjnej.
 * Klasa umożliwia dodawanie, edytowanie i usuwanie producentów w tabeli `vendors` w bazie danych,
 * a także synchronizację z głównym oknem aplikacji (`MainWindow`) w celu odświeżenia
 * combo boxów po zapisaniu zmian.
 *
 * @section Structure
 * Plik nagłówkowy zawiera:
 * 1. **Deklarację klasy vendors** – dziedziczy po QDialog.
 * 2. **Metody publiczne** – konstruktor, destruktor, ustawianie MainWindow.
 * 3. **Sloty prywatne** – obsługują akcje użytkownika (dodawanie, edycja, usuwanie, zatwierdzanie).
 * 4. **Metody prywatne** – odświeżanie listy producentów.
 * 5. **Zmienne prywatne** – przechowują interfejs, MainWindow i połączenie z bazą danych.
 *
 * @section Dependencies
 * - **Qt Framework**: Używa klas QDialog, QSqlDatabase.
 * - **Nagłówki aplikacji**: mainwindow.h (deklaracja MainWindow).
 * - **Namespace Ui**: Zawiera definicję interfejsu użytkownika (ui_vendors.h).
 *
 * @section Notes
 * - Kod nie został zmodyfikowany, zgodnie z wymaganiami użytkownika. Dodano jedynie komentarze i dokumentację.
 * - Klasa jest częścią systemu słownikowego, współpracuje z MainWindow do aktualizacji combo boxów.
 * - Obsługuje MySQL, ale aplikacja wspiera także SQLite (konfigurowane w DatabaseConfigDialog).
 */

#ifndef VENDORS_H
#define VENDORS_H

#include <QDialog>
#include <QSqlDatabase>

class MainWindow;

namespace Ui {
class vendors;
}

/**
 * @class vendors
 * @brief Okno dialogowe do zarządzania producentami sprzętu.
 *
 * @section ClassOverview
 * Klasa vendors dziedziczy po QDialog i zapewnia interfejs do dodawania, edytowania
 * i usuwania producentów sprzętu (np. Commodore, IBM) w bazie danych (tabela `vendors`).
 * Współpracuje z klasą MainWindow, aby odświeżyć listę producentów w combo boxie po zapisaniu zmian.
 *
 * @section Responsibilities
 * - Wyświetlanie listy producentów w QListView.
 * - Dodawanie nowych producentów do tabeli `vendors` z unikalnym UUID.
 * - Edycja istniejących producentów (zmiana nazwy).
 * - Usuwanie producentów po potwierdzeniu użytkownika.
 * - Odświeżanie combo boxa producentów w MainWindow po zapisaniu zmian.
 */
class vendors : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Konstruktor klasy vendors.
     * @param parent Wskaźnik na nadrzędny widget (domyślnie nullptr).
     *
     * @section ConstructorOverview
     * Inicjalizuje okno dialogowe, konfiguruje interfejs użytkownika,
     * ustanawia połączenie z bazą danych i podłącza sloty dla przycisków.
     */
    explicit vendors(QWidget *parent = nullptr);

    /**
     * @brief Destruktor klasy vendors.
     *
     * @section DestructorOverview
     * Zwalnia zasoby interfejsu użytkownika i usuwa obiekty dynamiczne.
     */
    ~vendors();

    /**
     * @brief Ustawia wskaźnik na główne okno aplikacji.
     * @param mainWindow Wskaźnik do instancji klasy MainWindow.
     *
     * @section MethodOverview
     * Umożliwia komunikację z głównym oknem w celu odświeżenia listy producentów
     * w combo boxie po zapisaniu zmian.
     */
    void setMainWindow(MainWindow *mainWindow);

private slots:
    /**
     * @brief Slot obsługujący dodanie nowego producenta.
     *
     * @section SlotOverview
     * Pobiera nazwę producenta z pola tekstowego, generuje UUID, zapisuje producenta
     * w tabeli `vendors` i odświeża listę.
     */
    void onAddClicked();

    /**
     * @brief Slot obsługujący edycję istniejącego producenta.
     *
     * @section SlotOverview
     * Pobiera wybranego producenta, otwiera okno dialogowe do edycji nazwy,
     * aktualizuje rekord w tabeli `vendors` i odświeża listę.
     */
    void onEditClicked();

    /**
     * @brief Slot obsługujący usunięcie zaznaczonego producenta.
     *
     * @section SlotOverview
     * Usuwa wybranego producenta z tabeli `vendors` po potwierdzeniu użytkownika
     * i odświeża listę.
     */
    void onDeleteClicked();

    /**
     * @brief Slot obsługujący zatwierdzenie zmian i zamknięcie okna.
     *
     * @section SlotOverview
     * Odświeża listę producentów w combo boxie MainWindow (jeśli ustawione),
     * zamyka okno dialogowe z wynikiem zaakceptowania.
     */
    void onOkClicked();

private:
    /**
     * @brief Odświeża widok listy producentów w interfejsie.
     *
     * @section MethodOverview
     * Pobiera producentów z tabeli `vendors`, sortuje alfabetycznie
     * i wyświetla w QListView za pomocą QSqlQueryModel.
     */
    void refreshList();

    /// Wskaźnik na interfejs użytkownika wygenerowany przez Qt Designer.
    Ui::vendors *ui;

    /// Wskaźnik na główne okno aplikacji.
    MainWindow *m_mainWindow;

    /// Obiekt połączenia z bazą danych.
    QSqlDatabase m_db;
};

#endif // VENDORS_H
