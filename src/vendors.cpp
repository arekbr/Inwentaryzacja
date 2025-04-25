/**
 * @file vendors.cpp
 * @brief Implementacja klasy vendors do zarządzania producentami sprzętu w aplikacji inwentaryzacyjnej.
 * @version 1.1.8
 * @date 2025-04-25
 * @author
 * - Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK
 * - ChatGPT
 * - GROK
 *
 * Klasa `vendors` umożliwia użytkownikowi zarządzanie listą producentów sprzętu.
 * Oferuje funkcje dodawania, edytowania i usuwania wpisów w tabeli `vendors`.
 * Po zatwierdzeniu zmian odświeża pole wyboru producenta w głównym oknie aplikacji.
 */

#include "vendors.h"
#include "ui_vendors.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlQueryModel>
#include <QInputDialog>
#include "mainwindow.h"
#include <QUuid> // dodane

/**
 * @brief Konstruktor klasy vendors.
 * @param parent Wskaźnik na widget nadrzędny (domyślnie nullptr).
 *
 * Inicjalizuje interfejs graficzny, ustawia połączenie z bazą danych
 * i podłącza przyciski do odpowiednich slotów. Po uruchomieniu wczytuje dane producentów.
 */
vendors::vendors(QWidget *parent)
    : QDialog(parent),
    ui(new Ui::vendors),
    m_mainWindow(nullptr)
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
 * Zwalnia zasoby interfejsu użytkownika.
 */
vendors::~vendors()
{
    delete ui;
}

/**
 * @brief Ustawia wskaźnik na główne okno aplikacji.
 * @param mainWindow Wskaźnik do instancji klasy MainWindow.
 *
 * Pozwala na synchronizację danych po zatwierdzeniu zmian.
 */
void vendors::setMainWindow(MainWindow *mainWindow)
{
    m_mainWindow = mainWindow;
}

/**
 * @brief Odświeża listę producentów w interfejsie użytkownika.
 *
 * Pobiera dane z tabeli `vendors` i ustawia je jako model dla `listView`.
 * W przypadku błędu SQL wyświetla komunikat krytyczny.
 */
void vendors::refreshList()
{
    QSqlQueryModel *queryModel = new QSqlQueryModel(this);
    queryModel->setQuery("SELECT name FROM vendors ORDER BY name ASC", m_db);
    if (queryModel->lastError().isValid()) {
        QMessageBox::critical(this, tr("Błąd"), tr("Błąd pobierania danych: %1")
                                                    .arg(queryModel->lastError().text()));
        return;
    }
    ui->listView->setModel(queryModel);
    ui->listView->setModelColumn(0);
}

/**
 * @brief Dodaje nowego producenta do bazy danych.
 *
 * Pobiera nazwę z pola tekstowego, generuje UUID i wstawia nowy rekord do tabeli `vendors`.
 * W przypadku błędu wykonania zapytania SQL wyświetla komunikat.
 */
void vendors::onAddClicked()
{
    QString newVendor = ui->lineEdit->text().trimmed();
    if(newVendor.isEmpty()){
        QMessageBox::warning(this, tr("Błąd"), tr("Nazwa producenta nie może być pusta."));
        return;
    }

    QString newId = QUuid::createUuid().toString(QUuid::WithoutBraces);

    QSqlQuery query(m_db);
    query.prepare("INSERT INTO vendors (id, name) VALUES (:id, :name)");
    query.bindValue(":id", newId);
    query.bindValue(":name", newVendor);
    if(!query.exec()){
        QMessageBox::critical(this, tr("Błąd"), tr("Nie udało się dodać producenta:\n%1")
                                                    .arg(query.lastError().text()));
    }
    refreshList();
    ui->lineEdit->clear();
}

/**
 * @brief Edytuje zaznaczonego producenta w bazie danych.
 *
 * Otwiera okno dialogowe z możliwością zmiany nazwy. Wyszukuje ID producenta i wykonuje aktualizację.
 * Jeśli wystąpi błąd — wyświetlany jest komunikat. Po zakończeniu odświeżana jest lista.
 */
void vendors::onEditClicked()
{
    QModelIndex index = ui->listView->currentIndex();
    if(!index.isValid()){
        QMessageBox::information(this, tr("Informacja"), tr("Proszę wybrać producenta do edycji."));
        return;
    }

    QString currentName = index.data(Qt::DisplayRole).toString();
    bool ok;
    QString newName = QInputDialog::getText(this, tr("Edytuj producenta"), tr("Nowa nazwa:"),
                                            QLineEdit::Normal, currentName, &ok);
    if(ok && !newName.trimmed().isEmpty()){
        QSqlQuery query(m_db);
        query.prepare("SELECT id FROM vendors WHERE name = :name");
        query.bindValue(":name", currentName);
        if(query.exec() && query.next()){
            QString id = query.value(0).toString();
            QSqlQuery updateQuery(m_db);
            updateQuery.prepare("UPDATE vendors SET name = :newName WHERE id = :id");
            updateQuery.bindValue(":newName", newName.trimmed());
            updateQuery.bindValue(":id", id);
            if(!updateQuery.exec()){
                QMessageBox::critical(this, tr("Błąd"),
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
 * Pyta użytkownika o potwierdzenie, a następnie wykonuje zapytanie DELETE.
 * W przypadku błędu wyświetla komunikat krytyczny.
 */
void vendors::onDeleteClicked()
{
    QModelIndex index = ui->listView->currentIndex();
    if(!index.isValid()){
        QMessageBox::information(this, tr("Informacja"), tr("Proszę wybrać producenta do usunięcia."));
        return;
    }

    QString vendorName = index.data(Qt::DisplayRole).toString();
    int ret = QMessageBox::question(this, tr("Potwierdzenie"),
                                    tr("Czy na pewno chcesz usunąć producenta: %1?").arg(vendorName),
                                    QMessageBox::Yes | QMessageBox::No);
    if(ret == QMessageBox::Yes){
        QSqlQuery query(m_db);
        query.prepare("DELETE FROM vendors WHERE name = :name");
        query.bindValue(":name", vendorName);
        if(!query.exec()){
            QMessageBox::critical(this, tr("Błąd"), tr("Nie udało się usunąć producenta:\n%1")
                                                        .arg(query.lastError().text()));
        }
    }
    refreshList();
}

/**
 * @brief Zamyka okno dialogowe i odświeża dane w głównym oknie.
 *
 * Jeśli `m_mainWindow` zostało ustawione, metoda odświeża combo box producentów.
 * Następnie zamyka okno (`accept()`).
 */
void vendors::onOkClicked()
{
    if(m_mainWindow) {
        m_mainWindow->loadComboBoxData("vendors", m_mainWindow->getNewItemVendorComboBox());
    }
    accept();
}
