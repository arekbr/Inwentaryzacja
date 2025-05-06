/**
 * @file models.h
 * @brief Deklaracja klasy models do zarządzania modelami eksponatów.
 * @author Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK & ChatGPT & GROK
 * @version \projectnumber
 * @date 2025-05-03
 *
 * @section Overview
 * Plik zawiera deklarację klasy models, która reprezentuje okno dialogowe do zarządzania
 * modelami eksponatów w aplikacji inwentaryzacyjnej. Klasa umożliwia dodawanie, edytowanie
 * i usuwanie modeli w tabeli 'models' w bazie danych, a także odświeżanie listy modeli
 * w głównym oknie aplikacji. Obsługuje identyfikator producenta (vendorId) jako QString.
 *
 * @section Structure
 * Plik nagłówkowy zawiera:
 * 1. **Deklarację klasy models** – dziedziczy po QDialog.
 * 2. **Metody publiczne** – konstruktor, destruktor, metody ustawiające MainWindow i vendorId.
 * 3. **Sloty prywatne** – obsługują akcje użytkownika (dodawanie, edycja, usuwanie, zatwierdzanie).
 * 4. **Metody prywatne** – odświeżanie listy modeli.
 * 5. **Zmienne prywatne** – przechowują interfejs, połączenie z bazą danych, MainWindow i vendorId.
 *
 * @section Dependencies
 * - **Qt Framework**: Używa klas QDialog, QSqlDatabase.
 * - **Nagłówki aplikacji**: mainwindow.h (deklaracja MainWindow).
 * - **Namespace Ui**: Zawiera definicję interfejsu użytkownika (ui_models.h).
 *
 * @section Notes
 * - Kod nie został zmodyfikowany, zgodnie z wymaganiami użytkownika. Dodano jedynie komentarze i dokumentację.
 * - Klasa jest częścią systemu słownikowego, współpracuje z MainWindow do aktualizacji combo boxów.
 * - Obsługuje MySQL, ale aplikacja wspiera także SQLite (konfigurowane w DatabaseConfigDialog).
 */

#ifndef MODELS_H
#define MODELS_H

#include <QDialog>
#include <QSqlDatabase>

class MainWindow;

namespace Ui {
class models;
}

/**
 * @class models
 * @brief Okno dialogowe do zarządzania modelami eksponatów.
 *
 * @section ClassOverview
 * Klasa models dziedziczy po QDialog i zapewnia interfejs do dodawania, edytowania
 * i usuwania modeli eksponatów w bazie danych. Współpracuje z klasą MainWindow,
 * aby odświeżyć listę modeli w combo boxie po zapisaniu zmian. Używa QString jako
 * typu dla identyfikatora producenta (vendorId).
 *
 * @section Responsibilities
 * - Wyświetlanie listy modeli w QListView.
 * - Dodawanie nowych modeli do tabeli 'models' z powiązaniem do producenta.
 * - Edycja istniejących modeli (zmiana nazwy).
 * - Usuwanie modeli po potwierdzeniu użytkownika.
 * - Odświeżanie combo boxa modeli w MainWindow po zapisaniu zmian.
 */
class models : public QDialog
{
    Q_OBJECT
public:
    /**
     * @brief Konstruktor klasy models.
     * @param parent Wskaźnik na nadrzędny widget. Domyślnie nullptr.
     *
     * @section ConstructorOverview
     * Inicjalizuje okno dialogowe, konfiguruje interfejs użytkownika,
     * ustanawia połączenie z bazą danych i podłącza sloty dla przycisków.
     */
    explicit models(QWidget *parent = nullptr);

    /**
     * @brief Destruktor klasy models.
     *
     * @section DestructorOverview
     * Zwalnia zasoby interfejsu użytkownika i usuwa obiekty dynamiczne.
     */
    ~models();

    /**
     * @brief Ustawia wskaźnik na główne okno aplikacji.
     * @param mainWindow Wskaźnik na obiekt MainWindow.
     *
     * @section MethodOverview
     * Umożliwia komunikację z głównym oknem w celu odświeżenia listy modeli
     * w combo boxie po zapisaniu zmian.
     */
    void setMainWindow(MainWindow *mainWindow);

    /**
     * @brief Ustawia identyfikator producenta dla modeli.
     * @param vendorId QString zawierający identyfikator producenta (np. UUID).
     *
     * @section MethodOverview
     * Ogranicza listę modeli do tych powiązanych z podanym producentem
     * i ustala vendorId dla nowych modeli.
     */
    void setVendorId(const QString &vendorId);

private slots:
    /**
     * @brief Dodaje nowy model do bazy danych.
     *
     * @section SlotOverview
     * Pobiera nazwę modelu z pola tekstowego, generuje UUID, zapisuje model
     * w tabeli 'models' z powiązaniem do producenta i odświeża listę.
     */
    void onAddClicked();

    /**
     * @brief Edytuje istniejący model w bazie danych.
     *
     * @section SlotOverview
     * Pobiera wybrany model, otwiera okno dialogowe do edycji nazwy,
     * aktualizuje rekord w tabeli 'models' i odświeża listę.
     */
    void onEditClicked();

    /**
     * @brief Usuwa wybrany model z bazy danych.
     *
     * @section SlotOverview
     * Usuwa wybrany model z tabeli 'models' po potwierdzeniu użytkownika
     * i odświeża listę.
     */
    void onDeleteClicked();

    /**
     * @brief Zamyka okno dialogowe i odświeża combo box modeli.
     *
     * @section SlotOverview
     * Odświeża listę modeli w combo boxie MainWindow (jeśli ustawione),
     * zamyka okno dialogowe z wynikiem zaakceptowania.
     */
    void onOkClicked();

private:
    /**
     * @brief Odświeża listę modeli w interfejsie.
     *
     * @section MethodOverview
     * Pobiera modele z tabeli 'models' dla ustawionego producenta,
     * wyświetla je w QListView za pomocą QSqlQueryModel.
     */
    void refreshList();

    /// Wskaźnik na obiekt interfejsu użytkownika.
    Ui::models *ui;
    /// Wskaźnik na główne okno aplikacji.
    MainWindow *m_mainWindow;
    /// Połączenie z bazą danych.
    QSqlDatabase m_db;
    /// Identyfikator producenta (np. UUID).
    QString m_vendorId;
};

#endif // MODELS_H
