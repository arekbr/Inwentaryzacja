/**
 * @file status.cpp
 * @brief Implementacja klasy status do zarządzania statusami eksponatów w aplikacji inwentaryzacyjnej.
 * @version 1.1.8
 * @date 2025-04-25
 * @author
 * - Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK
 * - ChatGPT
 * - GROK
 *
 * Plik zawiera implementację klasy `status`, która udostępnia użytkownikowi
 * interfejs dialogowy do dodawania, edytowania i usuwania statusów eksponatów
 * (np. "Nowy", "W użyciu", "Zniszczony"). Operacje wykonywane są na tabeli `statuses`
 * w bazie danych. Po zamknięciu okna, combo box w głównym oknie może zostać
 * odświeżony automatycznie.
 */

#include "status.h"
#include "ui_status.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlQueryModel>
#include <QInputDialog>
#include "mainwindow.h"
#include <QUuid>

/**
 * @brief Konstruktor klasy status.
 * @param parent Wskaźnik na nadrzędny widget, domyślnie nullptr.
 *
 * Inicjalizuje interfejs graficzny, konfiguruje połączenie z bazą danych
 * oraz ustawia sloty obsługujące przyciski. Po uruchomieniu wykonuje odświeżenie listy statusów.
 */
status::status(QWidget *parent)
    : QDialog(parent),
    ui(new Ui::status),
    m_mainWindow(nullptr)
{
    ui->setupUi(this);
    m_db = QSqlDatabase::database("default_connection");
    setWindowTitle(tr("Zarządzanie statusami"));

    connect(ui->pushButton_add, &QPushButton::clicked, this, &status::onAddClicked);
    connect(ui->pushButton_edit, &QPushButton::clicked, this, &status::onEditClicked);
    connect(ui->pushButton_delete, &QPushButton::clicked, this, &status::onDeleteClicked);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &status::onOkClicked);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    refreshList();
}

/**
 * @brief Destruktor klasy status.
 *
 * Usuwa interfejs użytkownika i zwalnia zasoby.
 */
status::~status()
{
    delete ui;
}

/**
 * @brief Ustawia wskaźnik na główne okno aplikacji.
 * @param mainWindow Wskaźnik do obiektu MainWindow.
 *
 * Pozwala na synchronizację statusów w combo boxach głównego okna po zatwierdzeniu zmian.
 */
void status::setMainWindow(MainWindow *mainWindow)
{
    m_mainWindow = mainWindow;
}

/**
 * @brief Odświeża listę statusów w interfejsie graficznym.
 *
 * Pobiera dane z tabeli `statuses` z bazy danych, sortuje je alfabetycznie
 * i ustawia jako model dla widoku listy.
 * W przypadku błędu SQL wyświetla komunikat krytyczny.
 */
void status::refreshList()
{
    QSqlQueryModel *queryModel = new QSqlQueryModel(this);
    queryModel->setQuery("SELECT name FROM statuses ORDER BY name ASC", m_db);
    if (queryModel->lastError().isValid()) {
        QMessageBox::critical(this, tr("Błąd"), tr("Błąd pobierania danych: %1")
                                                    .arg(queryModel->lastError().text()));
        return;
    }
    ui->listView->setModel(queryModel);
    ui->listView->setModelColumn(0);
}

/**
 * @brief Slot obsługujący dodawanie nowego statusu.
 *
 * Odczytuje wartość z pola tekstowego i dodaje nowy rekord do tabeli `statuses`,
 * generując automatycznie UUID jako identyfikator. Po udanym dodaniu odświeża widok.
 * W przypadku pustej nazwy lub błędu wykonania wyświetla odpowiedni komunikat.
 */
void status::onAddClicked()
{
    QString newStatus = ui->lineEdit->text().trimmed();
    if (newStatus.isEmpty()) {
        QMessageBox::warning(this, tr("Błąd"), tr("Nazwa statusu nie może być pusta."));
        return;
    }

    QUuid::createUuid().toString(QUuid::WithoutBraces);

    QSqlQuery query(m_db);
    query.prepare("INSERT INTO statuses (id, name) VALUES (:id, :name)");
    query.bindValue(":id", QUuid::createUuid().toString());
    query.bindValue(":name", newStatus);
    if (!query.exec()) {
        QMessageBox::critical(this, tr("Błąd"), tr("Nie udało się dodać statusu: %1")
                                                    .arg(query.lastError().text()));
    }
    refreshList();
    ui->lineEdit->clear();
}

/**
 * @brief Slot obsługujący edytowanie istniejącego statusu.
 *
 * Otwiera okno dialogowe z możliwością edycji wybranego statusu.
 * Wyszukuje ID statusu na podstawie nazwy, a następnie wykonuje aktualizację.
 * W przypadku niepowodzenia wyświetla komunikat o błędzie. Po zakończeniu
 * lista statusów jest odświeżana.
 */
void status::onEditClicked()
{
    QModelIndex index = ui->listView->currentIndex();
    if (!index.isValid()) {
        QMessageBox::information(this, tr("Informacja"), tr("Proszę wybrać status do edycji."));
        return;
    }

    QString currentName = index.data(Qt::DisplayRole).toString();
    bool ok;
    QString newName = QInputDialog::getText(this, tr("Edytuj status"), tr("Nowa nazwa:"), QLineEdit::Normal, currentName, &ok);
    if (ok && !newName.trimmed().isEmpty()) {
        QSqlQuery query(m_db);
        query.prepare("SELECT id FROM statuses WHERE name = :name");
        query.bindValue(":name", currentName);
        if (query.exec() && query.next()) {
            QString id = query.value("id").toString();
            QSqlQuery updateQuery(m_db);
            updateQuery.prepare("UPDATE statuses SET name = :newName WHERE id = :id");
            updateQuery.bindValue(":newName", newName.trimmed());
            updateQuery.bindValue(":id", id);
            if (!updateQuery.exec()) {
                QMessageBox::critical(this, tr("Błąd"), tr("Nie udało się zaktualizować statusu: %1")
                                                            .arg(updateQuery.lastError().text()));
            }
        }
    }

    refreshList();
}

/**
 * @brief Slot obsługujący usunięcie zaznaczonego statusu.
 *
 * Po potwierdzeniu przez użytkownika usuwa rekord z tabeli `statuses`.
 * W przypadku niepowodzenia wykonania zapytania wyświetla błąd.
 * Lista statusów jest odświeżana niezależnie od wyniku.
 */
void status::onDeleteClicked()
{
    QModelIndex index = ui->listView->currentIndex();
    if (!index.isValid()) {
        QMessageBox::information(this, tr("Informacja"), tr("Proszę wybrać status do usunięcia."));
        return;
    }

    QString statusName = index.data(Qt::DisplayRole).toString();
    int ret = QMessageBox::question(this, tr("Potwierdzenie"),
                                    tr("Czy na pewno chcesz usunąć status: %1?").arg(statusName),
                                    QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        QSqlQuery query(m_db);
        query.prepare("DELETE FROM statuses WHERE name = :name");
        query.bindValue(":name", statusName);
        if (!query.exec()) {
            QMessageBox::critical(this, tr("Błąd"), tr("Nie udało się usunąć statusu: %1")
                                                        .arg(query.lastError().text()));
        }
    }

    refreshList();
}

/**
 * @brief Slot obsługujący zatwierdzenie i zamknięcie okna.
 *
 * Jeśli ustawiono wskaźnik `m_mainWindow`, wywołuje metodę odświeżającą dane
 * w polu wyboru statusów w głównym oknie aplikacji.
 * Następnie zamyka okno dialogowe przez `accept()`.
 */
void status::onOkClicked()
{
    if (m_mainWindow) {
        m_mainWindow->loadComboBoxData("statuses", m_mainWindow->getNewItemStatusComboBox());
    }
    accept();
}
