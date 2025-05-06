/**
 * @file types.h
 * @brief Deklaracja klasy types służącej do zarządzania typami eksponatów w aplikacji inwentaryzacyjnej.
 * @version \projectnumber
 * @date 2025-05-03
 * @author Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK & ChatGPT & GROK
 *
 * @section Overview
 * Plik zawiera deklarację klasy types, która reprezentuje okno dialogowe do zarządzania
 * typami eksponatów (np. Komputer, Monitor, Drukarka) w aplikacji inwentaryzacyjnej.
 * Klasa umożliwia dodawanie, edytowanie i usuwanie typów w tabeli `types` w bazie danych,
 * a także synchronizację z głównym oknem aplikacji (`MainWindow`) w celu odświeżenia
 * combo boxów po zapisaniu zmian.
 *
 * @section Structure
 * Plik nagłówkowy zawiera:
 * 1. **Deklarację klasy types** – dziedziczy po QDialog.
 * 2. **Metody publiczne** – konstruktor, destruktor, ustawianie MainWindow.
 * 3. **Sloty prywatne** – obsługują akcje użytkownika (dodawanie, edycja, usuwanie, zatwierdzanie).
 * 4. **Metody prywatne** – odświeżanie listy typów.
 * 5. **Zmienne prywatne** – przechowują interfejs, MainWindow i połączenie z bazą danych.
 *
 * @section Dependencies
 * - **Qt Framework**: Używa klas QDialog, QSqlDatabase.
 * - **Nagłówki aplikacji**: mainwindow.h (deklaracja MainWindow).
 * - **Namespace Ui**: Zawiera definicję interfejsu użytkownika (ui_types.h).
 *
 * @section Notes
 * - Kod nie został zmodyfikowany, zgodnie z wymaganiami użytkownika. Dodano jedynie komentarze i dokumentację.
 * - Klasa jest częścią systemu słownikowego, współpracuje z MainWindow do aktualizacji combo boxów.
 * - Obsługuje MySQL, ale aplikacja wspiera także SQLite (konfigurowane w DatabaseConfigDialog).
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
 * @section ClassOverview
 * Klasa types dziedziczy po QDialog i zapewnia interfejs do dodawania, edytowania
 * i usuwania typów eksponatów (np. Komputer, Monitor) w bazie danych (tabela `types`).
 * Współpracuje z klasą MainWindow, aby odświeżyć listę typów w combo boxie po zapisaniu zmian.
 *
 * @section Responsibilities
 * - Wyświetlanie listy typów w QListView.
 * - Dodawanie nowych typów do tabeli `types` z unikalnym UUID.
 * - Edycja istniejących typów (zmiana nazwy).
 * - Usuwanie typów po potwierdzeniu użytkownika.
 * - Odświeżanie combo boxa typów w MainWindow po zapisaniu zmian.
 */
class types : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Konstruktor klasy types.
     * @param parent Wskaźnik na nadrzędny widget (domyślnie nullptr).
     *
     * @section ConstructorOverview
     * Inicjalizuje okno dialogowe, konfiguruje interfejs użytkownika,
     * ustanawia połączenie z bazą danych i podłącza sloty dla przycisków.
     */
    explicit types(QWidget *parent = nullptr);

    /**
     * @brief Destruktor klasy types.
     *
     * @section DestructorOverview
     * Zwalnia zasoby interfejsu użytkownika i usuwa obiekty dynamiczne.
     */
    ~types();

    /**
     * @brief Ustawia wskaźnik na główne okno aplikacji.
     * @param mainWindow Wskaźnik do obiektu klasy MainWindow.
     *
     * @section MethodOverview
     * Umożliwia komunikację z głównym oknem w celu odświeżenia listy typów
     * w combo boxie po zapisaniu zmian.
     */
    void setMainWindow(MainWindow *mainWindow);

private slots:
    /**
     * @brief Slot obsługujący dodawanie nowego typu.
     *
     * @section SlotOverview
     * Pobiera nazwę typu z pola tekstowego, generuje UUID, zapisuje typ
     * w tabeli `types` i odświeża listę.
     */
    void onAddClicked();

    /**
     * @brief Slot obsługujący edytowanie zaznaczonego typu.
     *
     * @section SlotOverview
     * Pobiera wybrany typ, otwiera okno dialogowe do edycji nazwy,
     * aktualizuje rekord w tabeli `types` i odświeża listę.
     */
    void onEditClicked();

    /**
     * @brief Slot obsługujący usunięcie zaznaczonego typu.
     *
     * @section SlotOverview
     * Usuwa wybrany typ z tabeli `types` po potwierdzeniu użytkownika
     * i odświeża listę.
     */
    void onDeleteClicked();

    /**
     * @brief Slot obsługujący zatwierdzenie zmian i zamknięcie okna.
     *
     * @section SlotOverview
     * Odświeża listę typów w combo boxie MainWindow (jeśli ustawione),
     * zamyka okno dialogowe z wynikiem zaakceptowania.
     */
    void onOkClicked();

private:
    /**
     * @brief Odświeża listę typów eksponatów w interfejsie.
     *
     * @section MethodOverview
     * Pobiera typy z tabeli `types`, sortuje alfabetycznie
     * i wyświetla w QListView za pomocą QSqlQueryModel.
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
