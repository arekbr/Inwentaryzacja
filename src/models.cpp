/**
 * @file models.cpp
 * @brief Implementacja klasy models do zarządzania modelami eksponatów.
 * @author Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK & ChatGPT & GROK
 * @version \projectnumber
 * @date 2025-05-03
 *
 * @section Overview
 * Plik zawiera implementację metod klasy models, odpowiedzialnej za zarządzanie modelami
 * eksponatów w bazie danych. Klasa umożliwia dodawanie, edytowanie i usuwanie modeli
 * w tabeli 'models', współpracując z interfejsem użytkownika (QListView, QLineEdit)
 * oraz klasą MainWindow w celu odświeżania combo boxów. Obsługuje powiązanie modeli
 * z producentami poprzez vendorId (QString).
 *
 * @section Structure
 * Kod jest podzielony na następujące sekcje:
 * 1. **Konstruktor** – inicjalizuje interfejs, połączenie z bazą danych, sloty.
 * 2. **Destruktor** – zwalnia zasoby.
 * 3. **Metody publiczne** – ustawianie MainWindow i vendorId.
 * 4. **Sloty prywatne** – obsługują akcje użytkownika (dodawanie, edycja, usuwanie, zatwierdzanie).
 * 5. **Metody prywatne** – odświeżanie listy modeli.
 *
 * @section Dependencies
 * - **Qt Framework**: Używa klas QMessageBox, QSqlQuery, QSqlQueryModel, QInputDialog, QUuid.
 * - **Nagłówki aplikacji**: models.h, mainwindow.h.
 * - **Interfejs użytkownika**: ui_models.h.
 *
 * @section Notes
 * - Kod nie został zmodyfikowany, zgodnie z wymaganiami użytkownika. Dodano jedynie komentarze i dokumentację.
 * - Klasa jest częścią systemu słownikowego, integruje się z MainWindow.
 * - Obsługuje MySQL, ale aplikacja wspiera także SQLite (konfigurowane w DatabaseConfigDialog).
 * - Lista modeli w QListView nie filtruje po vendorId, co może być rozszerzone w przyszłości.
 */

#include "models.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QUuid>
#include "mainwindow.h"
#include "ui_models.h"

/**
 * @brief Konstruktor klasy models.
 * @param parent Wskaźnik na nadrzędny widget (domyślnie nullptr).
 *
 * @section ConstructorOverview
 * Inicjalizuje interfejs graficzny, ustawia tytuł okna, nawiązuje połączenie
 * z bazą danych ('default_connection'), podłącza sloty dla przycisków
 * (dodaj, edytuj, usuń, OK, Anuluj) i odświeża listę modeli w QListView.
 */
models::models(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::models)
    , m_mainWindow(nullptr)
    , m_vendorId(QString()) // Pusty string jako "brak ID"
{
    ui->setupUi(this);
    m_db = QSqlDatabase::database("default_connection");
    setWindowTitle(tr("Zarządzanie modelami sprzętu"));

    // Połączenie przycisków
    connect(ui->pushButton_add, &QPushButton::clicked, this, &models::onAddClicked);
    connect(ui->pushButton_edit, &QPushButton::clicked, this, &models::onEditClicked);
    connect(ui->pushButton_delete, &QPushButton::clicked, this, &models::onDeleteClicked);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &models::onOkClicked);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    refreshList();
}

/**
 * @brief Destruktor klasy models.
 *
 * @section DestructorOverview
 * Usuwa obiekt interfejsu użytkownika i zwalnia zasoby.
 */
models::~models()
{
    delete ui;
}

/**
 * @brief Ustawia wskaźnik na główne okno aplikacji.
 * @param mainWindow Wskaźnik na obiekt MainWindow.
 *
 * @section MethodOverview
 * Przechowuje referencję do MainWindow, umożliwiając odświeżanie combo boxa
 * modeli po zapisaniu zmian.
 */
void models::setMainWindow(MainWindow *mainWindow)
{
    m_mainWindow = mainWindow;
}

/**
 * @brief Ustawia identyfikator producenta, powiązany z modelami.
 * @param vendorId QString zawierający identyfikator producenta (np. UUID).
 *
 * @section MethodOverview
 * Ustawia vendorId dla nowych modeli, używane podczas dodawania modelu
 * w metodzie onAddClicked.
 */
void models::setVendorId(const QString &vendorId)
{
    m_vendorId = vendorId;
}

/**
 * @brief Odświeża listę modeli w widoku listy.
 *
 * @section MethodOverview
 * Pobiera wszystkie modele z tabeli 'models', sortuje je alfabetycznie
 * i wyświetla w QListView za pomocą QSqlQueryModel. Wyświetla komunikat
 * o błędzie w przypadku niepowodzenia zapytania SQL.
 */
void models::refreshList()
{
    QSqlQueryModel *queryModel = new QSqlQueryModel(this);
    queryModel->setQuery("SELECT name FROM models ORDER BY name ASC", m_db);
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
 * @brief Slot obsługujący dodawanie nowego modelu.
 *
 * @section SlotOverview
 * Pobiera nazwę modelu z QLineEdit, generuje nowy UUID, wstawia model
 * do tabeli 'models' z powiązaniem do producenta (vendorId z m_vendorId
 * lub MainWindow). Wyświetla ostrzeżenie dla pustej nazwy lub błąd SQL.
 * Odświeża listę i czyści pole tekstowe po sukcesie.
 */
void models::onAddClicked()
{
    QString newModel = ui->lineEdit->text().trimmed();
    if (newModel.isEmpty()) {
        QMessageBox::warning(this, tr("Błąd"), tr("Nazwa modelu nie może być pusta."));
        return;
    }

    QString modelId = QUuid::createUuid().toString(QUuid::WithoutBraces);

    QSqlQuery query(m_db);
    query.prepare("INSERT INTO models (id, name, vendor_id) VALUES (:id, :name, :vendor_id)");
    query.bindValue(":id", modelId);

    QString vendorId;
    if (!m_vendorId.isEmpty()) {
        vendorId = m_vendorId;
    } else if (m_mainWindow) {
        vendorId = m_mainWindow->getNewItemVendorComboBox()->currentData().toString();
    }

    query.bindValue(":name", newModel);
    query.bindValue(":vendor_id", vendorId.isEmpty() ? QString("unknown_vendor_uuid") : vendorId);

    if (!query.exec()) {
        QMessageBox::critical(this,
                              tr("Błąd"),
                              tr("Nie udało się dodać modelu:\n%1").arg(query.lastError().text()));
    }

    refreshList();
    ui->lineEdit->clear();
}

/**
 * @brief Slot obsługujący edytowanie istniejącego modelu.
 *
 * @section SlotOverview
 * Pobiera wybrany model z QListView, otwiera QInputDialog do edycji nazwy,
 * aktualizuje rekord w tabeli 'models' za pomocą ID modelu. Wyświetla
 * informację, jeśli model nie jest wybrany, lub błąd SQL. Odświeża listę
 * po sukcesie.
 */
void models::onEditClicked()
{
    QModelIndex index = ui->listView->currentIndex();
    if (!index.isValid()) {
        QMessageBox::information(this, tr("Informacja"), tr("Proszę wybrać model do edycji."));
        return;
    }

    QString currentName = index.data(Qt::DisplayRole).toString();
    bool ok;
    QString newName = QInputDialog::getText(this,
                                            tr("Edytuj model"),
                                            tr("Nowa nazwa:"),
                                            QLineEdit::Normal,
                                            currentName,
                                            &ok);
    if (ok && !newName.trimmed().isEmpty()) {
        QSqlQuery query(m_db);
        query.prepare("SELECT id FROM models WHERE name = :name");
        query.bindValue(":name", currentName);
        if (query.exec() && query.next()) {
            QString id = query.value(0).toString();
            QSqlQuery updateQuery(m_db);
            updateQuery.prepare("UPDATE models SET name = :newName WHERE id = :id");
            updateQuery.bindValue(":newName", newName.trimmed());
            updateQuery.bindValue(":id", id);
            if (!updateQuery.exec()) {
                QMessageBox::critical(this,
                                      tr("Błąd"),
                                      tr("Nie udało się zaktualizować modelu:\n%1")
                                          .arg(updateQuery.lastError().text()));
            }
        }
    }

    refreshList();
}

/**
 * @brief Slot obsługujący usuwanie modelu.
 *
 * @section SlotOverview
 * Pobiera wybrany model z QListView, prosi o potwierdzenie usunięcia,
 * wykonuje zapytanie DELETE na tabeli 'models' dla nazwy modelu.
 * Wyświetla informację, jeśli model nie jest wybrany, lub błąd SQL.
 * Odświeża listę po każdej próbie.
 */
void models::onDeleteClicked()
{
    QModelIndex index = ui->listView->currentIndex();
    if (!index.isValid()) {
        QMessageBox::information(this, tr("Informacja"), tr("Proszę wybrać model do usunięcia."));
        return;
    }

    QString modelName = index.data(Qt::DisplayRole).toString();
    int ret = QMessageBox::question(this,
                                    tr("Potwierdzenie"),
                                    tr("Czy na pewno chcesz usunąć model: %1?").arg(modelName),
                                    QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        QSqlQuery query(m_db);
        query.prepare("DELETE FROM models WHERE name = :name");
        query.bindValue(":name", modelName);
        if (!query.exec()) {
            QMessageBox::critical(this,
                                  tr("Błąd"),
                                  tr("Nie udało się usunąć modelu:\n%1")
                                      .arg(query.lastError().text()));
        }
    }

    refreshList();
}

/**
 * @brief Slot obsługujący zatwierdzenie zmian i zamknięcie okna dialogowego.
 *
 * @section SlotOverview
 * Jeśli MainWindow jest ustawione, odświeża combo box modeli w głównym oknie
 * poprzez loadComboBoxData. Zamyka okno dialogowe z wynikiem akceptacji (accept).
 */
void models::onOkClicked()
{
    if (m_mainWindow) {
        m_mainWindow->loadComboBoxData("models", m_mainWindow->getNewItemModelComboBox());
    }
    accept();
}
