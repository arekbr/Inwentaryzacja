/**
 * @file status.cpp
 * @brief Implementacja klasy status do zarządzania statusami eksponatów w aplikacji inwentaryzacyjnej.
 * @version 1.2.2
 * @date 2025-05-03
 * @author Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK & ChatGPT & GROK
 *
 * @section Overview
 * Plik zawiera implementację metod klasy status, odpowiedzialnej za zarządzanie statusami
 * eksponatów (np. "Nowy", "Używany", "Uszkodzony") w bazie danych (tabela `statuses`).
 * Klasa umożliwia dodawanie, edytowanie i usuwanie statusów, współpracując z interfejsem
 * użytkownika (QListView, QLineEdit) oraz klasą MainWindow w celu odświeżania combo boxów.
 *
 * @section Structure
 * Kod jest podzielony na następujące sekcje:
 * 1. **Konstruktor** – inicjalizuje interfejs, połączenie z bazą danych, sloty.
 * 2. **Destruktor** – zwalnia zasoby.
 * 3. **Metody publiczne** – ustawianie MainWindow.
 * 4. **Sloty prywatne** – obsługują akcje użytkownika (dodawanie, edycja, usuwanie, zatwierdzanie).
 * 5. **Metody prywatne** – odświeżanie listy statusów.
 *
 * @section Dependencies
 * - **Qt Framework**: Używa klas QMessageBox, QSqlQuery, QSqlQueryModel, QInputDialog, QUuid.
 * - **Nagłówki aplikacji**: status.h, mainwindow.h.
 * - **Interfejs użytkownika**: ui_status.h.
 *
 * @section Notes
 * - Kod nie został zmodyfikowany, zgodnie z wymaganiami użytkownika. Dodano jedynie komentarze i dokumentację.
 * - Klasa jest częścią systemu słownikowego, integruje się z MainWindow.
 * - Obsługuje MySQL, ale aplikacja wspiera także SQLite (konfigurowane w DatabaseConfigDialog).
 * - Istnieje błąd w metodzie onAddClicked, gdzie generowany UUID nie jest przypisywany (naprawiony w dokumentacji).
 */

#include "status.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QUuid>
#include "mainwindow.h"
#include "ui_status.h"

/**
 * @brief Konstruktor klasy status.
 * @param parent Wskaźnik na nadrzędny widget, domyślnie nullptr.
 *
 * @section ConstructorOverview
 * Inicjalizuje interfejs graficzny, ustala połączenie z bazą danych
 * ('default_connection'), ustawia tytuł okna, podłącza sloty dla przycisków
 * (Dodaj, Edytuj, Usuń, OK, Anuluj) i odświeża listę statusów w QListView.
 */
status::status(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::status)
    , m_mainWindow(nullptr)
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
 * @section DestructorOverview
 * Usuwa obiekt interfejsu użytkownika i zwalnia zasoby.
 */
status::~status()
{
    delete ui;
}

/**
 * @brief Ustawia wskaźnik na główne okno aplikacji.
 * @param mainWindow Wskaźnik do obiektu MainWindow.
 *
 * @section MethodOverview
 * Przechowuje referencję do MainWindow, umożliwiając odświeżanie combo boxa
 * statusów po zapisaniu zmian.
 */
void status::setMainWindow(MainWindow *mainWindow)
{
    m_mainWindow = mainWindow;
}

/**
 * @brief Odświeża listę statusów w interfejsie graficznym.
 *
 * @section MethodOverview
 * Pobiera wszystkie statusy z tabeli `statuses`, sortuje je alfabetycznie
 * i wyświetla w QListView za pomocą QSqlQueryModel. Wyświetla komunikat
 * o błędzie w przypadku niepowodzenia zapytania SQL.
 */
void status::refreshList()
{
    QSqlQueryModel *queryModel = new QSqlQueryModel(this);
    queryModel->setQuery("SELECT name FROM statuses ORDER BY name ASC", m_db);
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
 * @brief Slot obsługujący dodawanie nowego statusu.
 *
 * @section SlotOverview
 * Pobiera nazwę statusu z QLineEdit, waliduje (niepusta), generuje UUID,
 * wstawia status do tabeli `statuses`. Wyświetla ostrzeżenie dla pustej nazwy
 * lub błąd SQL. Odświeża listę i czyści pole tekstowe po sukcesie.
 *
 * @section Bug
 * Linia `QUuid::createUuid().toString(QUuid::WithoutBraces);` generuje UUID,
 * ale wynik nie jest przypisywany. Poprawiona wersja powinna używać zmiennej
 * (np. `QString statusId = QUuid::createUuid().toString(QUuid::WithoutBraces);`).
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
        QMessageBox::critical(this,
                              tr("Błąd"),
                              tr("Nie udało się dodać statusu: %1").arg(query.lastError().text()));
    }
    refreshList();
    ui->lineEdit->clear();
}

/**
 * @brief Slot obsługujący edytowanie istniejącego statusu.
 *
 * @section SlotOverview
 * Pobiera wybrany status z QListView, otwiera QInputDialog do edycji nazwy,
 * aktualizuje rekord w tabeli `statuses` za pomocą ID statusu. Wyświetla
 * informację, jeśli status nie jest wybrany, lub błąd SQL. Odświeża listę
 * po sukcesie.
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
    QString newName = QInputDialog::getText(this,
                                            tr("Edytuj status"),
                                            tr("Nowa nazwa:"),
                                            QLineEdit::Normal,
                                            currentName,
                                            &ok);
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
                QMessageBox::critical(this,
                                      tr("Błąd"),
                                      tr("Nie udało się zaktualizować statusu: %1")
                                          .arg(updateQuery.lastError().text()));
            }
        }
    }

    refreshList();
}

/**
 * @brief Slot obsługujący usunięcie zaznaczonego statusu.
 *
 * @section SlotOverview
 * Pobiera wybrany status z QListView, prosi o potwierdzenie usunięcia,
 * wykonuje zapytanie DELETE na tabeli `statuses` dla nazwy statusu.
 * Wyświetla informację, jeśli status nie jest wybrany, lub błąd SQL.
 * Odświeża listę po każdej próbie.
 */
void status::onDeleteClicked()
{
    QModelIndex index = ui->listView->currentIndex();
    if (!index.isValid()) {
        QMessageBox::information(this, tr("Informacja"), tr("Proszę wybrać status do usunięcia."));
        return;
    }

    QString statusName = index.data(Qt::DisplayRole).toString();
    int ret = QMessageBox::question(this,
                                    tr("Potwierdzenie"),
                                    tr("Czy na pewno chcesz usunąć status: %1?").arg(statusName),
                                    QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        QSqlQuery query(m_db);
        query.prepare("DELETE FROM statuses WHERE name = :name");
        query.bindValue(":name", statusName);
        if (!query.exec()) {
            QMessageBox::critical(this,
                                  tr("Błąd"),
                                  tr("Nie udało się usunąć statusu: %1")
                                      .arg(query.lastError().text()));
        }
    }

    refreshList();
}

/**
 * @brief Slot obsługujący zatwierdzenie i zamknięcie okna.
 *
 * @section SlotOverview
 * Jeśli MainWindow jest ustawione, odświeża combo box statusów w głównym oknie
 * poprzez loadComboBoxData. Zamyka okno dialogowe z wynikiem akceptacji (accept).
 */
void status::onOkClicked()
{
    if (m_mainWindow) {
        m_mainWindow->loadComboBoxData("statuses", m_mainWindow->getNewItemStatusComboBox());
    }
    accept();
}
