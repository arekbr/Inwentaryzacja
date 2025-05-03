/**
 * @file storage.cpp
 * @brief Implementacja klasy storage służącej do zarządzania miejscami przechowywania eksponatów w aplikacji inwentaryzacyjnej.
 * @version 1.2.2
 * @date 2025-05-03
 * @author Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK & ChatGPT & GROK
 *
 * @section Overview
 * Plik zawiera implementację metod klasy storage, odpowiedzialnej za zarządzanie
 * miejscami przechowywania eksponatów (np. magazyny, półki, pokoje) w bazie danych
 * (tabela `storage_places`). Klasa umożliwia dodawanie, edytowanie i usuwanie lokalizacji,
 * współpracując z interfejsem użytkownika (QListView, QLineEdit) oraz klasą MainWindow
 * w celu odświeżania combo boxów po zapisaniu zmian.
 *
 * @section Structure
 * Kod jest podzielony na następujące sekcje:
 * 1. **Konstruktor** – inicjalizuje interfejs, połączenie z bazą danych, sloty.
 * 2. **Destruktor** – zwalnia zasoby.
 * 3. **Metody publiczne** – ustawianie MainWindow.
 * 4. **Sloty prywatne** – obsługują akcje użytkownika (dodawanie, edycja, usuwanie, zatwierdzanie).
 * 5. **Metody prywatne** – odświeżanie listy lokalizacji.
 *
 * @section Dependencies
 * - **Qt Framework**: Używa klas QMessageBox, QSqlQuery, QSqlQueryModel, QInputDialog, QUuid.
 * - **Nagłówki aplikacji**: storage.h, mainwindow.h.
 * - **Interfejs użytkownika**: ui_storage.h.
 *
 * @section Notes
 * - Kod nie został zmodyfikowany, zgodnie z wymaganiami użytkownika. Dodano jedynie komentarze i dokumentację.
 * - Klasa jest częścią systemu słownikowego, integruje się z MainWindow.
 * - Obsługuje MySQL, ale aplikacja wspiera także SQLite (konfigurowane w DatabaseConfigDialog).
 * - Istnieje błąd w metodzie onAddClicked, gdzie generowany UUID nie jest przypisywany (naprawiony w dokumentacji).
 */

#include "storage.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QUuid>
#include "mainwindow.h"
#include "ui_storage.h"

/**
 * @brief Konstruktor klasy storage.
 * @param parent Wskaźnik na widget nadrzędny (domyślnie nullptr).
 *
 * @section ConstructorOverview
 * Inicjalizuje interfejs graficzny, ustala połączenie z bazą danych
 * ('default_connection'), ustawia tytuł okna, podłącza sloty dla przycisków
 * (Dodaj, Edytuj, Usuń, OK, Anuluj) i odświeża listę lokalizacji w QListView.
 */
storage::storage(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::storage)
    , m_mainWindow(nullptr)
{
    ui->setupUi(this);
    m_db = QSqlDatabase::database("default_connection");

    setWindowTitle(tr("Zarządzanie miejscami przechowywania"));

    connect(ui->pushButton_add, &QPushButton::clicked, this, &storage::onAddClicked);
    connect(ui->pushButton_edit, &QPushButton::clicked, this, &storage::onEditClicked);
    connect(ui->pushButton_delete, &QPushButton::clicked, this, &storage::onDeleteClicked);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &storage::onOkClicked);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    refreshList();
}

/**
 * @brief Destruktor klasy storage.
 *
 * @section DestructorOverview
 * Usuwa obiekt interfejsu użytkownika i zwalnia zasoby.
 */
storage::~storage()
{
    delete ui;
}

/**
 * @brief Ustawia wskaźnik na główne okno aplikacji.
 * @param mainWindow Wskaźnik do instancji klasy MainWindow.
 *
 * @section MethodOverview
 * Przechowuje referencję do MainWindow, umożliwiając odświeżanie combo boxa
 * lokalizacji po zapisaniu zmian.
 */
void storage::setMainWindow(MainWindow *mainWindow)
{
    m_mainWindow = mainWindow;
}

/**
 * @brief Odświeża widok listy lokalizacji w interfejsie użytkownika.
 *
 * @section MethodOverview
 * Pobiera wszystkie lokalizacje z tabeli `storage_places`, sortuje je alfabetycznie
 * i wyświetla w QListView za pomocą QSqlQueryModel. Wyświetla komunikat
 * o błędzie w przypadku niepowodzenia zapytania SQL.
 */
void storage::refreshList()
{
    QSqlQueryModel *queryModel = new QSqlQueryModel(this);
    queryModel->setQuery("SELECT name FROM storage_places ORDER BY name ASC", m_db);
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
 * @brief Obsługuje dodawanie nowej lokalizacji magazynowej.
 *
 * @section SlotOverview
 * Pobiera nazwę lokalizacji z QLineEdit, waliduje (niepusta), generuje UUID,
 * wstawia lokalizację do tabeli `storage_places`. Wyświetla ostrzeżenie dla pustej nazwy
 * lub błąd SQL. Odświeża listę i czyści pole tekstowe po sukcesie.
 *
 * @section Bug
 * Linia `QUuid::createUuid().toString(QUuid::WithoutBraces);` generuje UUID,
 * ale wynik nie jest przypisywany. Poprawiona wersja powinna używać zmiennej
 * (np. `QString storageId = QUuid::createUuid().toString(QUuid::WithoutBraces);`).
 */
void storage::onAddClicked()
{
    QString newStorage = ui->lineEdit->text().trimmed();
    if (newStorage.isEmpty()) {
        QMessageBox::warning(this,
                             tr("Błąd"),
                             tr("Nazwa miejsca przechowywania nie może być pusta."));
        return;
    }

    QUuid::createUuid().toString(QUuid::WithoutBraces);

    QSqlQuery query(m_db);
    query.prepare("INSERT INTO storage_places (id, name) VALUES (:id, :name)");
    query.bindValue(":id", QUuid::createUuid().toString());
    query.bindValue(":name", newStorage);
    if (!query.exec()) {
        QMessageBox::critical(this,
                              tr("Błąd"),
                              tr("Nie udało się dodać miejsca przechowywania: %1")
                                  .arg(query.lastError().text()));
    }
    refreshList();
    ui->lineEdit->clear();
}

/**
 * @brief Obsługuje edycję aktualnie zaznaczonej lokalizacji.
 *
 * @section SlotOverview
 * Pobiera wybraną lokalizację z QListView, otwiera QInputDialog do edycji nazwy,
 * aktualizuje rekord w tabeli `storage_places` za pomocą ID lokalizacji. Wyświetla
 * informację, jeśli lokalizacja nie jest wybrana, lub błąd SQL. Odświeża listę
 * po sukcesie.
 */
void storage::onEditClicked()
{
    QModelIndex index = ui->listView->currentIndex();
    if (!index.isValid()) {
        QMessageBox::information(this,
                                 tr("Informacja"),
                                 tr("Proszę wybrać miejsce przechowywania do edycji."));
        return;
    }

    QString currentName = index.data(Qt::DisplayRole).toString();
    bool ok;
    QString newName = QInputDialog::getText(this,
                                            tr("Edytuj miejsce przechowywania"),
                                            tr("Nowa nazwa:"),
                                            QLineEdit::Normal,
                                            currentName,
                                            &ok);
    if (ok && !newName.trimmed().isEmpty()) {
        QSqlQuery query(m_db);
        query.prepare("SELECT id FROM storage_places WHERE name = :name");
        query.bindValue(":name", currentName);
        if (query.exec() && query.next()) {
            QString id = query.value("id").toString();
            QSqlQuery updateQuery(m_db);
            updateQuery.prepare("UPDATE storage_places SET name = :newName WHERE id = :id");
            updateQuery.bindValue(":newName", newName.trimmed());
            updateQuery.bindValue(":id", id);
            if (!updateQuery.exec()) {
                QMessageBox::critical(this,
                                      tr("Błąd"),
                                      tr("Nie udało się zaktualizować miejsca przechowywania: %1")
                                          .arg(updateQuery.lastError().text()));
            }
        }
    }

    refreshList();
}

/**
 * @brief Obsługuje usunięcie zaznaczonej lokalizacji.
 *
 * @section SlotOverview
 * Pobiera wybraną lokalizację z QListView, prosi o potwierdzenie usunięcia,
 * wykonuje zapytanie DELETE na tabeli `storage_places` dla nazwy lokalizacji.
 * Wyświetla informację, jeśli lokalizacja nie jest wybrana, lub błąd SQL.
 * Odświeża listę po każdej próbie.
 */
void storage::onDeleteClicked()
{
    QModelIndex index = ui->listView->currentIndex();
    if (!index.isValid()) {
        QMessageBox::information(this,
                                 tr("Informacja"),
                                 tr("Proszę wybrać miejsce przechowywania do usunięcia."));
        return;
    }

    QString storageName = index.data(Qt::DisplayRole).toString();
    int ret = QMessageBox::question(this,
                                    tr("Potwierdzenie"),
                                    tr("Czy na pewno chcesz usunąć miejsce przechowywania: %1?")
                                        .arg(storageName),
                                    QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        QSqlQuery query(m_db);
        query.prepare("DELETE FROM storage_places WHERE name = :name");
        query.bindValue(":name", storageName);
        if (!query.exec()) {
            QMessageBox::critical(this,
                                  tr("Błąd"),
                                  tr("Nie udało się usunąć miejsca przechowywania: %1")
                                      .arg(query.lastError().text()));
        }
    }

    refreshList();
}

/**
 * @brief Obsługuje zamknięcie okna i synchronizację danych.
 *
 * @section SlotOverview
 * Jeśli MainWindow jest ustawione, odświeża combo box lokalizacji w głównym oknie
 * poprzez loadComboBoxData. Zamyka okno dialogowe z wynikiem akceptacji (accept).
 */
void storage::onOkClicked()
{
    if (m_mainWindow) {
        m_mainWindow->loadComboBoxData("storage_places",
                                       m_mainWindow->getNewItemStoragePlaceComboBox());
    }
    accept();
}
