/**
 * @file vendors.h
 * @brief Deklaracja klasy vendors do zarządzania producentami sprzętu w aplikacji inwentaryzacyjnej.
 * @version 1.1.8
 * @date 2025-04-25
 * @author
 * - Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK
 * - ChatGPT
 * - GROK
 *
 * Plik zawiera deklarację klasy `vendors`, reprezentującej okno dialogowe
 * umożliwiające użytkownikowi zarządzanie producentami sprzętu (np. Commodore, IBM, Apple).
 * Klasa współpracuje z `MainWindow`, umożliwiając synchronizację danych po modyfikacjach.
 *
 * Operacje wykonywane są na tabeli `vendors` w bazie danych.
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
 * Klasa dziedziczy po QDialog i umożliwia dodawanie, edytowanie oraz usuwanie
 * producentów z bazy danych. Integruje się z głównym oknem aplikacji w celu
 * automatycznego odświeżania list wyboru.
 */
class vendors : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Konstruktor klasy vendors.
     * @param parent Wskaźnik na nadrzędny widget (domyślnie nullptr).
     *
     * Inicjalizuje interfejs graficzny i ustanawia połączenie z bazą danych.
     */
    explicit vendors(QWidget *parent = nullptr);

    /**
     * @brief Destruktor klasy vendors.
     *
     * Zwalnia zasoby interfejsu użytkownika.
     */
    ~vendors();

    /**
     * @brief Ustawia wskaźnik na główne okno aplikacji.
     * @param mainWindow Wskaźnik do instancji klasy MainWindow.
     *
     * Pozwala na synchronizację danych po zatwierdzeniu zmian.
     */
    void setMainWindow(MainWindow *mainWindow);

private slots:
    /**
     * @brief Slot obsługujący dodanie nowego producenta.
     *
     * Wywoływany po kliknięciu przycisku „Dodaj”.
     */
    void onAddClicked();

    /**
     * @brief Slot obsługujący edycję istniejącego producenta.
     *
     * Wywoływany po kliknięciu przycisku „Edytuj”.
     */
    void onEditClicked();

    /**
     * @brief Slot obsługujący usunięcie zaznaczonego producenta.
     *
     * Wywoływany po kliknięciu przycisku „Usuń”.
     */
    void onDeleteClicked();

    /**
     * @brief Slot obsługujący zatwierdzenie zmian i zamknięcie okna.
     *
     * Wywoływany po kliknięciu przycisku „OK”. Odświeża listę producentów w combo boxie głównego okna.
     */
    void onOkClicked();

private:
    /**
     * @brief Odświeża widok listy producentów w interfejsie.
     *
     * Pobiera dane z tabeli `vendors` i ustawia je w komponencie listy.
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
