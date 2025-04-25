/**
 * @file storage.cpp
 * @brief Implementacja klasy storage służącej do zarządzania miejscami przechowywania eksponatów w aplikacji inwentaryzacyjnej.
 * @version 1.1.8
 * @date 2025-04-25
 * @author
 * - Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK
 * - ChatGPT
 * - GROK
 *
 * Klasa `storage` realizuje interfejs dialogowy umożliwiający użytkownikowi dodawanie,
 * edytowanie i usuwanie lokalizacji magazynowych z tabeli `storage_places` w bazie danych.
 * Po zatwierdzeniu zmian możliwe jest odświeżenie danych w głównym oknie aplikacji.
 */

#include "storage.h"
#include "ui_storage.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlQueryModel>
#include <QInputDialog>
#include "mainwindow.h"
#include <QUuid>

/**
 * @brief Konstruktor klasy storage.
 * @param parent Wskaźnik na widget nadrzędny (domyślnie nullptr).
 *
 * Inicjalizuje interfejs użytkownika, konfiguruje połączenie z bazą danych,
 * ustawia tytuł okna i podłącza sygnały przycisków do odpowiednich slotów.
 * Na końcu wywołuje `refreshList()` w celu załadowania danych.
 */
storage::storage(QWidget *parent)
    : QDialog(parent),
    ui(new Ui::storage),
    m_mainWindow(nullptr)
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
 * Umożliwia synchronizację combo boxa lokalizacji po zatwierdzeniu zmian.
 */
void storage::setMainWindow(MainWindow *mainWindow)
{
    m_mainWindow = mainWindow;
}

/**
 * @brief Odświeża widok listy lokalizacji w interfejsie użytkownika.
 *
 * Pobiera dane z tabeli `storage_places` i ustawia je jako model dla `listView`.
 * W razie błędu wykonania zapytania SQL wyświetla komunikat o błędzie.
 */
void storage::refreshList()
{
    QSqlQueryModel *queryModel = new QSqlQueryModel(this);
    queryModel->setQuery("SELECT name FROM storage_places ORDER BY name ASC", m_db);
    if (queryModel->lastError().isValid()) {
        QMessageBox::critical(this, tr("Błąd"), tr("Błąd pobierania danych: %1")
                                                    .arg(queryModel->lastError().text()));
        return;
    }
    ui->listView->setModel(queryModel);
    ui->listView->setModelColumn(0);
}

/**
 * @brief Obsługuje dodawanie nowej lokalizacji magazynowej.
 *
 * Pobiera nazwę z pola tekstowego i dodaje rekord do tabeli `storage_places`,
 * generując automatycznie UUID jako klucz główny. W razie błędu informuje użytkownika.
 */
void storage::onAddClicked()
{
    QString newStorage = ui->lineEdit->text().trimmed();
    if (newStorage.isEmpty()) {
        QMessageBox::warning(this, tr("Błąd"), tr("Nazwa miejsca przechowywania nie może być pusta."));
        return;
    }

    QUuid::createUuid().toString(QUuid::WithoutBraces);

    QSqlQuery query(m_db);
    query.prepare("INSERT INTO storage_places (id, name) VALUES (:id, :name)");
    query.bindValue(":id", QUuid::createUuid().toString());
    query.bindValue(":name", newStorage);
    if (!query.exec()) {
        QMessageBox::critical(this, tr("Błąd"), tr("Nie udało się dodać miejsca przechowywania: %1")
                                                    .arg(query.lastError().text()));
    }
    refreshList();
    ui->lineEdit->clear();
}

/**
 * @brief Obsługuje edycję aktualnie zaznaczonej lokalizacji.
 *
 * Otwiera okno dialogowe z możliwością zmiany nazwy. Następnie aktualizuje rekord
 * w tabeli `storage_places` na podstawie ID pobranego z bazy.
 */
void storage::onEditClicked()
{
    QModelIndex index = ui->listView->currentIndex();
    if (!index.isValid()) {
        QMessageBox::information(this, tr("Informacja"), tr("Proszę wybrać miejsce przechowywania do edycji."));
        return;
    }

    QString currentName = index.data(Qt::DisplayRole).toString();
    bool ok;
    QString newName = QInputDialog::getText(this, tr("Edytuj miejsce przechowywania"), tr("Nowa nazwa:"), QLineEdit::Normal, currentName, &ok);
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
                QMessageBox::critical(this, tr("Błąd"), tr("Nie udało się zaktualizować miejsca przechowywania: %1")
                                                            .arg(updateQuery.lastError().text()));
            }
        }
    }

    refreshList();
}

/**
 * @brief Obsługuje usunięcie zaznaczonej lokalizacji.
 *
 * Pyta użytkownika o potwierdzenie, a następnie wykonuje zapytanie DELETE.
 * W przypadku błędu informuje użytkownika i odświeża listę lokalizacji.
 */
void storage::onDeleteClicked()
{
    QModelIndex index = ui->listView->currentIndex();
    if (!index.isValid()) {
        QMessageBox::information(this, tr("Informacja"), tr("Proszę wybrać miejsce przechowywania do usunięcia."));
        return;
    }

    QString storageName = index.data(Qt::DisplayRole).toString();
    int ret = QMessageBox::question(this, tr("Potwierdzenie"),
                                    tr("Czy na pewno chcesz usunąć miejsce przechowywania: %1?").arg(storageName),
                                    QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        QSqlQuery query(m_db);
        query.prepare("DELETE FROM storage_places WHERE name = :name");
        query.bindValue(":name", storageName);
        if (!query.exec()) {
            QMessageBox::critical(this, tr("Błąd"), tr("Nie udało się usunąć miejsca przechowywania: %1")
                                                        .arg(query.lastError().text()));
        }
    }

    refreshList();
}

/**
 * @brief Obsługuje zamknięcie okna i synchronizację danych.
 *
 * Jeśli ustawiono `m_mainWindow`, wywołuje metodę do odświeżenia combo boxa
 * z lokalizacjami. Następnie zamyka okno dialogowe.
 */
void storage::onOkClicked()
{
    if (m_mainWindow) {
        m_mainWindow->loadComboBoxData("storage_places", m_mainWindow->getNewItemStoragePlaceComboBox());
    }
    accept();
}
