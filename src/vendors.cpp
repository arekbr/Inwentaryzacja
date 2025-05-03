/**
 * @file vendors.cpp
 * @brief Implementacja klasy vendors do zarządzania producentami sprzętu w aplikacji inwentaryzacyjnej.
 * @version 1.2.2
 * @date 2025-05-03
 * @author Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK & ChatGPT & GROK
 *
 * @section Overview
 * Plik zawiera implementację metod klasy vendors, odpowiedzialnej za zarządzanie
 * producentami sprzętu (np. Commodore, IBM, Apple) w bazie danych (tabela `vendors`).
 * Klasa umożliwia dodawanie, edytowanie i usuwanie producentów, współpracując z interfejsem
 * użytkownika (QListView, QLineEdit) oraz klasą MainWindow w celu odświeżania combo boxów
 * po zapisaniu zmian.
 *
 * @section Structure
 * Kod jest podzielony na następujące sekcje:
 * 1. **Konstruktor** – inicjalizuje interfejs, połączenie z bazą danych, sloty.
 * 2. **Destruktor** – zwalnia zasoby.
 * 3. **Metody publiczne** – ustawianie MainWindow.
 * 4. **Sloty prywatne** – obsługują akcje użytkownika (dodawanie, edycja, usuwanie, zatwierdzanie).
 * 5. **Metody prywatne** – odświeżanie listy producentów.
 *
 * @section Dependencies
 * - **Qt Framework**: Używa klas QMessageBox, QSqlQuery, QSqlQueryModel, QInputDialog, QUuid.
 * - **Nagłówki aplikacji**: vendors.h, mainwindow.h.
 * - **Interfejs użytkownika**: ui_vendors.h.
 *
 * @section Notes
 * - Kod nie został zmodyfikowany, zgodnie z wymaganiami użytkownika. Dodano jedynie komentarze i dokumentację.
 * - Klasa jest częścią systemu słownikowego, integruje się z MainWindow.
 * - Obsługuje MySQL, ale aplikacja wspiera także SQLite (konfigurowane w DatabaseConfigDialog).
 */

#include "vendors.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QUuid> // dodane
#include "mainwindow.h"
#include "ui_vendors.h"

/**
 * @brief Konstruktor klasy vendors.
 * @param parent Wskaźnik na widget nadrzędny (domyślnie nullptr).
 *
 * @section ConstructorOverview
 * Inicjalizuje interfejs graficzny, ustala połączenie z bazą danych
 * ('default_connection'), ustawia tytuł okna, podłącza sloty dla przycisków
 * (Dodaj, Edytuj, Usuń, OK, Anuluj) i odświeża listę producentów w QListView.
 */
vendors::vendors(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::vendors)
    , m_mainWindow(nullptr)
{
    ui->setupUi(this);
    m_db = QSqlDatabase::database("default_connection");
    setWindowTitle(tr("Zarządzanie producentami"));

    // Połączenie przycisków
    connect(ui->pushButton_add, &QPushButton::clicked, this, &vendors::onAddClicked);
    connect(ui->pushButton_edit, &QPushButton::clicked, this, &vendors::onEditClicked);
    connect(ui->pushButton_delete, &QPushButton::clicked, this, &vendors::onDeleteClicked);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &vendors::onOkClicked);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    refreshList();
}

/**
 * @brief Destruktor klasy vendors.
 *
 * @section DestructorOverview
 * Usuwa obiekt interfejsu użytkownika i zwalnia zasoby.
 */
vendors::~vendors()
{
    delete ui;
}

/**
 * @brief Ustawia wskaźnik na główne okno aplikacji.
 * @param mainWindow Wskaźnik do instancji klasy MainWindow.
 *
 * @section MethodOverview
 * Przechowuje referencję do MainWindow, umożliwiając odświeżanie combo boxa
 * producentów po zapisaniu zmian.
 */
void vendors::setMainWindow(MainWindow *mainWindow)
{
    m_mainWindow = mainWindow;
}

/**
 * @brief Odświeża listę producentów w interfejsie użytkownika.
 *
 * @section MethodOverview
 * Pobiera wszystkich producentów z tabeli `vendors`, sortuje ich alfabetycznie
 * i wyświetla w QListView za pomocą QSqlQueryModel. Wyświetla komunikat
 * o błędzie w przypadku niepowodzenia zapytania SQL.
 */
void vendors::refreshList()
{
    QSqlQueryModel *queryModel = new QSqlQueryModel(this);
    queryModel->setQuery("SELECT name FROM vendors ORDER BY name ASC", m_db);
    if (queryModel->lastError().isValid()) {
        QMessageBox::critical(this,
                              tr("Błąd"),
                              tr("Błąd pobierania danych: %1").arg(queryModel->lastError().text()));
        return;
    }
    ui->listView->setModel(queryModel);
    ui->listView->setModelColumn(0);
}

/**
 * @brief Dodaje nowego producenta do bazy danych.
 *
 * @section SlotOverview
 * Pobiera nazwę producenta z QLineEdit, waliduje (niepusta), generuje UUID,
 * wstawia producenta do tabeli `vendors`. Wyświetla ostrzeżenie dla pustej nazwy
 * lub błąd SQL. Odświeża listę i czyści pole tekstowe po sukcesie.
 */
void vendors::onAddClicked()
{
    QString newVendor = ui->lineEdit->text().trimmed();
    if (newVendor.isEmpty()) {
        QMessageBox::warning(this, tr("Błąd"), tr("Nazwa producenta nie może być pusta."));
        return;
    }

    QString newId = QUuid::createUuid().toString(QUuid::WithoutBraces);

    QSqlQuery query(m_db);
    query.prepare("INSERT INTO vendors (id, name) VALUES (:id, :name)");
    query.bindValue(":id", newId);
    query.bindValue(":name", newVendor);
    if (!query.exec()) {
        QMessageBox::critical(this,
                              tr("Błąd"),
                              tr("Nie udało się dodać producenta:\n%1")
                                  .arg(query.lastError().text()));
    }
    refreshList();
    ui->lineEdit->clear();
}

/**
 * @brief Edytuje zaznaczonego producenta w bazie danych.
 *
 * @section SlotOverview
 * Pobiera wybranego producenta z QListView, otwiera QInputDialog do edycji nazwy,
 * aktualizuje rekord w tabeli `vendors` za pomocą ID producenta. Wyświetla
 * informację, jeśli producent nie jest wybrany, lub błąd SQL. Odświeża listę
 * po sukcesie.
 */
void vendors::onEditClicked()
{
    QModelIndex index = ui->listView->currentIndex();
    if (!index.isValid()) {
        QMessageBox::information(this, tr("Informacja"), tr("Proszę wybrać producenta do edycji."));
        return;
    }

    QString currentName = index.data(Qt::DisplayRole).toString();
    bool ok;
    QString newName = QInputDialog::getText(this,
                                            tr("Edytuj producenta"),
                                            tr("Nowa nazwa:"),
                                            QLineEdit::Normal,
                                            currentName,
                                            &ok);
    if (ok && !newName.trimmed().isEmpty()) {
        QSqlQuery query(m_db);
        query.prepare("SELECT id FROM vendors WHERE name = :name");
        query.bindValue(":name", currentName);
        if (query.exec() && query.next()) {
            QString id = query.value(0).toString();
            QSqlQuery updateQuery(m_db);
            updateQuery.prepare("UPDATE vendors SET name = :newName WHERE id = :id");
            updateQuery.bindValue(":newName", newName.trimmed());
            updateQuery.bindValue(":id", id);
            if (!updateQuery.exec()) {
                QMessageBox::critical(this,
                                      tr("Błąd"),
                                      tr("Nie udało się zaktualizować producenta:\n%1")
                                          .arg(updateQuery.lastError().text()));
            }
        }
    }
    refreshList();
}

/**
 * @brief Usuwa zaznaczonego producenta z bazy danych.
 *
 * @section SlotOverview
 * Pobiera wybranego producenta z QListView, prosi o potwierdzenie usunięcia,
 * wykonuje zapytanie DELETE na tabeli `vendors` dla nazwy producenta.
 * Wyświetla informację, jeśli producent nie jest wybrany, lub błąd SQL.
 * Odświeża listę po każdej próbie.
 */
void vendors::onDeleteClicked()
{
    QModelIndex index = ui->listView->currentIndex();
    if (!index.isValid()) {
        QMessageBox::information(this,
                                 tr("Informacja"),
                                 tr("Proszę wybrać producenta do usunięcia."));
        return;
    }

    QString vendorName = index.data(Qt::DisplayRole).toString();
    int ret = QMessageBox::question(this,
                                    tr("Potwierdzenie"),
                                    tr("Czy na pewno chcesz usunąć producenta: %1?").arg(vendorName),
                                    QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        QSqlQuery query(m_db);
        query.prepare("DELETE FROM vendors WHERE name = :name");
        query.bindValue(":name", vendorName);
        if (!query.exec()) {
            QMessageBox::critical(this,
                                  tr("Błąd"),
                                  tr("Nie udało się usunąć producenta:\n%1")
                                      .arg(query.lastError().text()));
        }
    }
    refreshList();
}

/**
 * @brief Zamyka okno dialogowe i odświeża dane w głównym oknie.
 *
 * @section SlotOverview
 * Jeśli MainWindow jest ustawione, odświeża combo box producentów w głównym oknie
 * poprzez loadComboBoxData. Zamyka okno dialogowe z wynikiem akceptacji (accept).
 */
void vendors::onOkClicked()
{
    if (m_mainWindow) {
        m_mainWindow->loadComboBoxData("vendors", m_mainWindow->getNewItemVendorComboBox());
    }
    accept();
}
