/**
 * @file types.cpp
 * @brief Implementacja klasy types do zarządzania typami eksponatów w aplikacji inwentaryzacyjnej.
 * @version 1.1.8
 * @date 2025-04-25
 * @author
 * - Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK
 * - ChatGPT
 * - GROK
 *
 * Plik zawiera implementację klasy `types`, która dostarcza interfejs graficzny
 * do zarządzania typami sprzętu w aplikacji inwentaryzacyjnej.
 * Umożliwia dodawanie, edytowanie i usuwanie rekordów w tabeli `types` w bazie danych.
 * Współpracuje z `MainWindow`, umożliwiając odświeżenie pól wyboru typów.
 */

#include "types.h"
#include "ui_types.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlQueryModel>
#include <QInputDialog>
#include "mainwindow.h"
#include <QUuid>

/**
 * @brief Konstruktor klasy types.
 * @param parent Wskaźnik na nadrzędny widget (domyślnie nullptr).
 *
 * Inicjalizuje interfejs użytkownika, ustawia połączenie z bazą danych
 * i podłącza przyciski do odpowiednich slotów. Wywołuje `refreshList()`.
 */
types::types(QWidget *parent)
    : QDialog(parent),
    ui(new Ui::types),
    m_mainWindow(nullptr)
{
    ui->setupUi(this);
    m_db = QSqlDatabase::database("default_connection"); // MySQL
    setWindowTitle(tr("Zarządzanie typami sprzętu"));

    connect(ui->pushButton_add,    &QPushButton::clicked, this, &types::onAddClicked);
    connect(ui->pushButton_edit,   &QPushButton::clicked, this, &types::onEditClicked);
    connect(ui->pushButton_delete, &QPushButton::clicked, this, &types::onDeleteClicked);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &types::onOkClicked);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    refreshList();
}

/**
 * @brief Destruktor klasy types.
 *
 * Zwalnia zasoby interfejsu użytkownika.
 */
types::~types()
{
    delete ui;
}

/**
 * @brief Ustawia wskaźnik na główne okno aplikacji.
 * @param mainWindow Wskaźnik do instancji klasy MainWindow.
 *
 * Pozwala na synchronizację danych z głównym oknem po zatwierdzeniu zmian.
 */
void types::setMainWindow(MainWindow *mainWindow)
{
    m_mainWindow = mainWindow;
}

/**
 * @brief Odświeża listę typów wyświetlanych w widoku listy.
 *
 * Pobiera dane z tabeli `types` i ustawia model danych dla `listView`.
 * W razie błędu wykonania zapytania SQL wyświetlany jest komunikat o błędzie.
 */
void types::refreshList()
{
    QSqlQueryModel *model = new QSqlQueryModel(this);
    model->setQuery("SELECT name FROM types ORDER BY name ASC", m_db);
    if (model->lastError().isValid()) {
        QMessageBox::critical(this, tr("Błąd"), tr("Błąd pobierania (MySQL): %1")
                                                    .arg(model->lastError().text()));
        return;
    }
    ui->listView->setModel(model);
    ui->listView->setModelColumn(0);
}

/**
 * @brief Dodaje nowy typ do bazy danych.
 *
 * Pobiera tekst z pola `type_lineEdit`, generuje UUID i dodaje rekord
 * do tabeli `types`. W przypadku błędu informuje użytkownika.
 */
void types::onAddClicked()
{
    QString txt = ui->type_lineEdit->text().trimmed();
    if (txt.isEmpty()) {
        QMessageBox::warning(this, tr("Błąd"), tr("Nazwa typu nie może być pusta."));
        return;
    }
    QString uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);

    QSqlQuery q(m_db);
    q.prepare("INSERT INTO types (id, name) VALUES (:id, :name)");
    q.bindValue(":id", uuid);
    q.bindValue(":name", txt);

    if (!q.exec()) {
        QMessageBox::critical(this, tr("Błąd"),
                              tr("Nie udało się dodać:\n%1").arg(q.lastError().text()));
    }
    refreshList();
    ui->type_lineEdit->clear();
}

/**
 * @brief Edytuje aktualnie zaznaczony typ.
 *
 * Otwiera okno dialogowe z możliwością zmiany nazwy.
 * Aktualizuje rekord w bazie danych na podstawie ID typu.
 */
void types::onEditClicked()
{
    QModelIndex idx = ui->listView->currentIndex();
    if (!idx.isValid()) {
        QMessageBox::information(this, tr("Informacja"), tr("Wybierz rekord do edycji."));
        return;
    }
    QString oldName = idx.data(Qt::DisplayRole).toString();
    bool ok;
    QString newName = QInputDialog::getText(this, tr("Edytuj typ"),
                                            tr("Nowa nazwa:"),
                                            QLineEdit::Normal,
                                            oldName, &ok);
    if (ok && !newName.trimmed().isEmpty()) {
        QSqlQuery q(m_db);
        q.prepare("SELECT id FROM types WHERE name=:nm");
        q.bindValue(":nm", oldName);
        if (q.exec() && q.next()) {
            QString id = q.value(0).toString();
            QSqlQuery q2(m_db);
            q2.prepare("UPDATE types SET name=:n WHERE id=:id");
            q2.bindValue(":n", newName.trimmed());
            q2.bindValue(":id", id);
            if (!q2.exec()) {
                QMessageBox::critical(this, tr("Błąd"),
                                      tr("Nie udało się zaktualizować:\n%1")
                                          .arg(q2.lastError().text()));
            }
        }
    }
    refreshList();
}

/**
 * @brief Usuwa wybrany typ z bazy danych.
 *
 * Pyta użytkownika o potwierdzenie i wykonuje zapytanie DELETE.
 * Po zakończeniu operacji odświeża listę typów.
 */
void types::onDeleteClicked()
{
    QModelIndex idx = ui->listView->currentIndex();
    if (!idx.isValid()) {
        QMessageBox::information(this, tr("Informacja"), tr("Wybierz rekord do usunięcia."));
        return;
    }
    QString name = idx.data(Qt::DisplayRole).toString();
    auto ans = QMessageBox::question(this, tr("Potwierdzenie"),
                                     tr("Usunąć typ %1?").arg(name),
                                     QMessageBox::Yes|QMessageBox::No);
    if (ans==QMessageBox::Yes) {
        QSqlQuery q(m_db);
        q.prepare("DELETE FROM types WHERE name=:n");
        q.bindValue(":n", name);
        if (!q.exec()) {
            QMessageBox::critical(this, tr("Błąd"),
                                  tr("Nie udało się usunąć:\n%1")
                                      .arg(q.lastError().text()));
        }
    }
    refreshList();
}

/**
 * @brief Zamyka okno dialogowe i odświeża dane w głównym oknie.
 *
 * Jeśli `m_mainWindow` jest ustawione, odświeża combo box typów.
 * Następnie wywołuje `accept()`.
 */
void types::onOkClicked()
{
    if (m_mainWindow) {
        m_mainWindow->loadComboBoxData("types", m_mainWindow->getNewItemTypeComboBox());
    }
    accept();
}
