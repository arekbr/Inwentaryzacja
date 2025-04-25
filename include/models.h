/**
 * @file models.h
 * @brief Deklaracja klasy models do zarządzania modelami eksponatów.
 * @author Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK & ChatGPT & GROK
 * @version 1.1.8
 * @date 2025-04-25
 *
 * Plik zawiera deklarację klasy models, która reprezentuje okno dialogowe do zarządzania
 * modelami eksponatów w aplikacji inwentaryzacyjnej. Klasa umożliwia dodawanie, edytowanie
 * i usuwanie modeli w tabeli 'models' w bazie danych, a także odświeżanie listy modeli
 * w głównym oknie aplikacji. Obsługuje identyfikator producenta (vendorId) jako QString.
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
 * Klasa models dziedziczy po QDialog i zapewnia interfejs do dodawania, edytowania
 * i usuwania modeli eksponatów w bazie danych. Współpracuje z klasą MainWindow,
 * aby odświeżyć listę modeli w combo boxie po zapisaniu zmian. Używa QString jako
 * typu dla identyfikatora producenta (vendorId).
 */
class models : public QDialog
{
    Q_OBJECT
public:
    /**
     * @brief Konstruktor klasy models.
     * @param parent Wskaźnik na nadrzędny widget. Domyślnie nullptr.
     *
     * Inicjalizuje okno dialogowe i konfiguruje interfejs użytkownika.
     */
    explicit models(QWidget *parent = nullptr);

    /**
     * @brief Destruktor klasy models.
     *
     * Zwalnia zasoby interfejsu użytkownika.
     */
    ~models();

    /**
     * @brief Ustawia wskaźnik na główne okno aplikacji.
     * @param mainWindow Wskaźnik na obiekt MainWindow.
     *
     * Umożliwia komunikację z głównym oknem w celu odświeżenia listy modeli w combo boxie.
     */
    void setMainWindow(MainWindow *mainWindow);

    /**
     * @brief Ustawia identyfikator producenta dla modeli.
     * @param vendorId QString zawierający identyfikator producenta (np. UUID).
     *
     * Ogranicza listę modeli do tych powiązanych z podanym producentem.
     */
    void setVendorId(const QString &vendorId);

private slots:
    /**
     * @brief Dodaje nowy model do bazy danych.
     *
     * Pobiera nazwę modelu z interfejsu, zapisuje ją w tabeli 'models' i odświeża listę.
     */
    void onAddClicked();

    /**
     * @brief Edytuje istniejący model w bazie danych.
     *
     * Pobiera wybrany model, umożliwia zmianę jego nazwy i aktualizuje rekord w tabeli 'models'.
     */
    void onEditClicked();

    /**
     * @brief Usuwa wybrany model z bazy danych.
     *
     * Usuwa wybrany model z tabeli 'models' po potwierdzeniu i odświeża listę.
     */
    void onDeleteClicked();

    /**
     * @brief Zamyka okno dialogowe i odświeża combo box modeli.
     *
     * Zapisuje zmiany, odświeża listę modeli w głównym oknie i zamyka okno dialogowe.
     */
    void onOkClicked();

private:
    /**
     * @brief Odświeża listę modeli w interfejsie.
     *
     * Pobiera modele z tabeli 'models' dla ustawionego producenta i wyświetla je w interfejsie.
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
