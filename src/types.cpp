/**
 * @file types.cpp
 * @brief Implementacja klasy types do zarządzania typami eksponatów w aplikacji inwentaryzacyjnej.
 * @version \projectnumber
 * @date 2025-05-03
 * @author Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK & ChatGPT & GROK
 *
 * @section Overview
 * Plik zawiera implementację metod klasy types, odpowiedzialnej za zarządzanie
 * typami eksponatów (np. Komputer, Monitor, Drukarka) w bazie danych (tabela `types`).
 * Klasa umożliwia dodawanie, edytowanie i usuwanie typów, współpracując z interfejsem
 * użytkownika (QListView, QLineEdit) oraz klasą MainWindow w celu odświeżania combo boxów
 * po zapisaniu zmian.
 *
 * @section Structure
 * Kod jest podzielony na następujące sekcje:
 * 1. **Konstruktor** – inicjalizuje interfejs, połączenie z bazą danych, sloty.
 * 2. **Destruktor** – zwalnia zasoby.
 * 3. **Metody publiczne** – ustawianie MainWindow.
 * 4. **Sloty prywatne** – obsługują akcje użytkownika (dodawanie, edycja, usuwanie, zatwierdzanie).
 * 5. **Metody prywatne** – odświeżanie listy typów.
 *
 * @section Dependencies
 * - **Qt Framework**: Używa klas QMessageBox, QSqlQuery, QSqlQueryModel, QInputDialog, QUuid.
 * - **Nagłówki aplikacji**: types.h, mainwindow.h.
 * - **Interfejs użytkownika**: ui_types.h.
 *
 * @section Notes
 * - Kod nie został zmodyfikowany, zgodnie z wymaganiami użytkownika. Dodano jedynie komentarze i dokumentację.
 * - Klasa jest częścią systemu słownikowego, integruje się z MainWindow.
 * - Obsługuje MySQL, ale aplikacja wspiera także SQLite (konfigurowane w DatabaseConfigDialog).
 */

#include "types.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QUuid>
#include "mainwindow.h"
#include "ui_types.h"

/**
 * @brief Konstruktor klasy types.
 * @param parent Wskaźnik na nadrzędny widget (domyślnie nullptr).
 *
 * @section ConstructorOverview
 * Inicjalizuje interfejs graficzny, ustala połączenie z bazą danych
 * ('default_connection'), ustawia tytuł okna, podłącza sloty dla przycisków
 * (Dodaj, Edytuj, Usuń, OK, Anuluj) i odświeża listę typów w QListView.
 */
types::types(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::types)
    , m_mainWindow(nullptr)
{
    ui->setupUi(this);
    m_db = QSqlDatabase::database("default_connection"); // MySQL
    setWindowTitle(tr("Zarządzanie typami sprzętu"));

    connect(ui->pushButton_add, &QPushButton::clicked, this, &types::onAddClicked);
    connect(ui->pushButton_edit, &QPushButton::clicked, this, &types::onEditClicked);
    connect(ui->pushButton_delete, &QPushButton::clicked, this, &types::onDeleteClicked);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &types::onOkClicked);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    refreshList();
}

/**
 * @brief Destruktor klasy types.
 *
 * @section DestructorOverview
 * Usuwa obiekt interfejsu użytkownika i zwalnia zasoby.
 */
types::~types()
{
    delete ui;
}

/**
 * @brief Ustawia wskaźnik na główne okno aplikacji.
 * @param mainWindow Wskaźnik do instancji klasy MainWindow.
 *
 * @section MethodOverview
 * Przechowuje referencję do MainWindow, umożliwiając odświeżanie combo boxa
 * typów po zapisaniu zmian.
 */
void types::setMainWindow(MainWindow *mainWindow)
{
    m_mainWindow = mainWindow;
}

/**
 * @brief Odświeża listę typów wyświetlanych w widoku listy.
 *
 * @section MethodOverview
 * Pobiera wszystkie typy z tabeli `types`, sortuje je alfabetycznie
 * i wyświetla w QListView za pomocą QSqlQueryModel. Wyświetla komunikat
 * o błędzie w przypadku niepowodzenia zapytania SQL.
 */
void types::refreshList()
{
    QSqlQueryModel *model = new QSqlQueryModel(this);
    model->setQuery("SELECT name FROM types ORDER BY name ASC", m_db);
    if (model->lastError().isValid()) {
        QMessageBox::critical(this,
                              tr("Błąd"),
                              tr("Błąd pobierania (MySQL): %1").arg(model->lastError().text()));
        return;
    }
    ui->listView->setModel(model);
    ui->listView->setModelColumn(0);
}

/**
 * @brief Dodaje nowy typ do bazy danych.
 *
 * @section SlotOverview
 * Pobiera nazwę typu z QLineEdit, waliduje (niepusta), generuje UUID,
 * wstawia typ do tabeli `types`. Wyświetla ostrzeżenie dla pustej nazwy
 * lub błąd SQL. Odświeża listę i czyści pole tekstowe po sukcesie.
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
        QMessageBox::critical(this,
                              tr("Błąd"),
                              tr("Nie udało się dodać:\n%1").arg(q.lastError().text()));
    }
    refreshList();
    ui->type_lineEdit->clear();
}

/**
 * @brief Edytuje aktualnie zaznaczony typ.
 *
 * @section SlotOverview
 * Pobiera wybrany typ z QListView, otwiera QInputDialog do edycji nazwy,
 * aktualizuje rekord w tabeli `types` za pomocą ID typu. Wyświetla
 * informację, jeśli typ nie jest wybrany, lub błąd SQL. Odświeża listę
 * po sukcesie.
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
    QString newName = QInputDialog::getText(this,
                                            tr("Edytuj typ"),
                                            tr("Nowa nazwa:"),
                                            QLineEdit::Normal,
                                            oldName,
                                            &ok);
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
                QMessageBox::critical(this,
                                      tr("Błąd"),
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
 * @section SlotOverview
 * Pobiera wybrany typ z QListView, prosi o potwierdzenie usunięcia,
 * wykonuje zapytanie DELETE na tabeli `types` dla nazwy typu.
 * Wyświetla informację, jeśli typ nie jest wybrany, lub błąd SQL.
 * Odświeża listę po każdej próbie.
 */
void types::onDeleteClicked()
{
    QModelIndex idx = ui->listView->currentIndex();
    if (!idx.isValid()) {
        QMessageBox::information(this, tr("Informacja"), tr("Wybierz rekord do usunięcia."));
        return;
    }
    QString name = idx.data(Qt::DisplayRole).toString();
    auto ans = QMessageBox::question(this,
                                     tr("Potwierdzenie"),
                                     tr("Usunąć typ %1?").arg(name),
                                     QMessageBox::Yes | QMessageBox::No);
    if (ans == QMessageBox::Yes) {
        QSqlQuery q(m_db);
        q.prepare("DELETE FROM types WHERE name=:n");
        q.bindValue(":n", name);
        if (!q.exec()) {
            QMessageBox::critical(this,
                                  tr("Błąd"),
                                  tr("Nie udało się usunąć:\n%1").arg(q.lastError().text()));
        }
    }
    refreshList();
}

/**
 * @brief Zamyka okno dialogowe i odświeża dane w głównym oknie.
 *
 * @section SlotOverview
 * Jeśli MainWindow jest ustawione, odświeża combo box typów w głównym oknie
 * poprzez loadComboBoxData. Zamyka okno dialogowe z wynikiem akceptacji (accept).
 */
void types::onOkClicked()
{
    if (m_mainWindow) {
        m_mainWindow->loadComboBoxData("types", m_mainWindow->getNewItemTypeComboBox());
    }
    accept();
}
